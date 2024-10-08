#include "mainwindow.h"
#include "ui_mainwindow.h"

// Libs for Interface
#include <QFileDialog>
// Working with Files
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardItem>
#include <QModelIndex>
#include <QProcess>
#include <QMessageBox>
#include <QPushButton>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _lines = {ui->x_le, ui->y_le, ui->z_le, ui->Mom_x_le, ui->Mom_y_le, ui->Mom_z_le};
    _labels = {ui->x_lbl, ui->y_lbl, ui->z_lbl, ui->Mom_x_lbl, ui->Mom_y_lbl, ui->Mom_z_lbl};
    _flags = {ui->x_ChBx, ui->y_ChBx, ui->z_ChBx, ui->Mom_x_ChBx, ui->Mom_y_ChBx, ui->Mom_z_CheBx};
    _model = new QStandardItemModel(ui->treeView);
    ui->treeView->setModel(_model);

    for(const auto& line: _lines)
    {line->setValidator(new QIntValidator);}
    ui->displ_nt_le->setValidator(new QIntValidator);
    ui->pres_le->setValidator(new QIntValidator);
    Hide();
    ui->Set_changes_Bn->hide();
    ui->parameters->hide();
    ui->Set_changes_Bn->setToolTip("Set changes");
    ui->Save_file_Bn->setToolTip("save to new file");
    ui->find_file_Bn->setToolTip("select file");
    statusBar()->showMessage("select file to start");
    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::TakeItemFromTree);
    connect(ui->Set_changes_Bn, &QPushButton::clicked, this, &MainWindow::SetChanges);
    connect(ui->Save_file_Bn, &QPushButton::clicked, this, &MainWindow::SaveToFile);
    connect(ui->find_file_Bn, &QPushButton::clicked, this, &MainWindow::FindFile);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::FindConditions()
{
    _read_file = {};
    if(!_doc.contains("restraints")) {
        qCritical() << "no restraints";
    }
    else{
        QJsonArray restraints;
        for (const auto restraint : _doc["restraints"].toArray()){
            QJsonObject sub_restraints;
            sub_restraints.insert("type", 0);
            sub_restraints.insert("name", "resrtraint");
            sub_restraints.insert("id", restraint.toObject()["id"].toInt());
            restraints.append(sub_restraints);
        }
    _read_file.insert("restraints", restraints);
    }
    if(_doc["loads"].isNull()){
        qCritical() << "no loads";
    }
    else{
    QJsonArray force, dis_force, pressure;
    for (const auto& load : _doc["loads"].toArray()){
        QJsonObject subload;
        subload.insert("type", load.toObject()["type"].toInt());
        subload.insert("name", load.toObject()["name"].toString());
        subload.insert("id", load.toObject()["id"].toInt());
        switch(load.toObject()["type"].toInt()){
        case 3: {pressure.append(subload);break;}
        case 5: {force.append(subload);break;}
        case 35: {dis_force.append(subload);break;}
        default: {statusBar()->showMessage("uncnoun load");}
        }
    }
    if (!force.isEmpty()) {_read_file.insert("force",force);}
    if (!pressure.isEmpty()) {_read_file.insert("pressure",pressure);}
    if (!dis_force.isEmpty()) {_read_file.insert("distributed force",dis_force);}
        }
    NewTree();
}

void MainWindow::Hide() const
{
    for(int i = 0; i < 6; i++) {
        _lines[i]->hide();
        _flags[i]->hide();
        _labels[i]->hide();
    }
    ui->pres_le->hide();
    ui->pres_lbl->hide();
    ui->displ_nt_lbl->hide();
    ui->displ_nt_le->hide();
}

void MainWindow::Display() const
{
    if(_buffer.isEmpty()){
        return;
    }
    ui->Set_changes_Bn->show(); ui->parameters->show();
    if(_buffer.contains("flag")) {
        ui->displ_nt_lbl->show();
        ui->displ_nt_le->show();
        ui->displ_nt_le->setText("0");
        for(int i = 0; i < 6 ;i++) {
            _flags[i]->show();
            _flags[i]->setChecked(_buffer.value("flag")[i].toInt());
            _labels[i]->show();
            if(_buffer["data"][i][0]!=0) {
                ui->displ_nt_le->setText(QString::number(_buffer.value("data")[i][0].toDouble()));
            }
        }
    return;}
    if(_buffer["data"].toArray().size() > 1) {
        for(int i = 0; i < 6 ;i++){
        _lines[i]->show();
        _lines[i]->setText(QString::number(_buffer.value("data")[i][0].toDouble()));
        _labels[i]->show();}
    }else{
        ui->pres_le->show();
        ui->pres_le->setText(QString::number(_buffer.value("data")[0][0].toDouble()));
        ui->pres_lbl->show();
    }
}

void MainWindow::TakeItemFromTree(const QModelIndex &index)
{
    if (!index.parent().isValid()) {
        return;}
    _item =  _model->itemFromIndex(index);
    if (!(_item->data(1).toInt() == 0)) {
        for(const auto& value : _doc["loads"].toArray()){
            QJsonObject iterator = value.toObject();
            if (iterator["id"].toInt()==_item->data(3).toInt()&&iterator["type"].toInt()==_item->data(1).toInt()){
                _buffer = iterator;
                break;
            }
        }
    }else{
        for(const auto& value : _doc["restraints"].toArray()){
            QJsonObject iterator = value.toObject();
            if (iterator["id"]==_item->data(3).toInt()){
                _buffer = iterator;
                break;
            }
        }
    }
    Hide();
    Display();
}

void MainWindow::TakeChanges()
{
    if(_item->data(1).toInt() == 0) {
        QJsonArray r_array =  _doc["restraints"].toArray();
        for( int i = 0; i < r_array.size();i++) {
            if (r_array[i].toObject()["id"]==_item->data(3).toInt()){
                r_array[i] = QJsonValue(_buffer);
                _doc.insert("restraints",r_array);
                statusBar()->showMessage("Changes for" + _item->data(2).toString() +  _item->data(3).toString() + " saved");
                return;}
        }
    }
    QJsonArray l_array = _doc["loads"].toArray();
    for( int i = 0; i < l_array.size();i++) {
        if (l_array[i].toObject()["id"]==_buffer["id"] && l_array[i].toObject()["name"] ==_buffer["name"]){
            l_array[i] = QJsonValue(QJsonObject(_buffer));
            _doc["loads"] = l_array;
            statusBar()->showMessage("Changes for" + _item->data(2).toString() +  _item->data(3).toString() + " saved");
            return;}
    }
}

void MainWindow::FindFile()
{
    _file_path = QFileDialog::getOpenFileName(this,
                                    "Открыть файл",
                                    "",
                                    "Fidesys file (*.fc)");
    if (_file_path.isEmpty()){
        statusBar()->showMessage("file is Empty");
        return;
    }
    QFile file(_file_path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, QString("%1").arg(_file_path),
                             QString("Can't open file %1\nError: %2.")
                                 .arg(_file_path)
                                 .arg(file.errorString()));
        return;
    }

    _doc = (QJsonDocument::fromJson(file.readAll())).object();
    _model->clear();
    FindConditions();
    Hide();
    ui->Set_changes_Bn->hide();
    ui->parameters->hide();
    statusBar()->showMessage("file " + _file_path.section("/",-1) +  " is open");
    ui->filename_lbl->setText(_file_path.section("/",-1));
}

void MainWindow::SetChanges()
{
    QJsonArray data, flag, pres, subpres, displace;
    for(int i = 0; i < 6; i++){
        QJsonArray subdata, subdisplace;
        subdisplace.append(0.0);
        displace.append(subdisplace);
        subdata.append(_lines[i]->text().toDouble());
        data.append(subdata);
        flag.append(_flags[i]->isChecked()?1:0);
    }
    QJsonArray subdis;
    subdis.append(ui->displ_nt_le->text().toDouble());
    displace.pop_back();
    displace.append(subdis);
    subpres.append(ui->pres_le->text().toDouble());
    pres.append(subpres);
    switch (_item->data(1).toInt()) {
    case 0:     {_buffer.insert("flag", flag);_buffer.insert("data", displace);break;}
    case 3:     {_buffer.insert("data", pres);break;}
    default:    {_buffer.insert("data", data);break;}
    }
    TakeChanges();
}

bool MainWindow::SaveToFile()
{

    QString file_name = QFileDialog::getSaveFileName(
                                                this,
                                                "Сохранить файл",
                                                _file_path,
                                                "Fidesys file (*.fc)");
    QFile new_file(file_name);
    if (!new_file.open(QIODevice::WriteOnly))
    {
        qCritical() << "Can't write new .fc file";
        statusBar()->showMessage("Can't write new .fc file");
        return false;
    }
    new_file.write(QJsonDocument(_doc).toJson(QJsonDocument::Indented));
    new_file.close();
return true;
}

void MainWindow::NewTree()
{
    _model->setHorizontalHeaderLabels(QStringList() << "data" << "id");
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        for(const auto& imap: _loads){
        if(_read_file.contains(imap.second))
        {
            QStandardItem *item = new QStandardItem(imap.second);
            for(const auto& value : _read_file[imap.second].toArray())
            {
                QStandardItem *sub = new QStandardItem();
                sub->setData(QString::number(value.toObject()["type"].toInt()),1);
                sub->setData(value.toObject()["name"].toString(),2);
                sub->setData(QString::number(value.toObject()["id"].toInt()),3);
                sub->setText(value.toObject()["name"].toString());
                item->setChild(value.toObject()["id"].toInt()-1,sub);
                item->setChild(value.toObject()["id"].toInt()-1,1,new QStandardItem(QString::number(value.toObject()["id"].toInt())));
                sub->setFlags(item->flags() & ~Qt::ItemIsEditable);
            }
            _model->appendRow(item);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }
    }
}


