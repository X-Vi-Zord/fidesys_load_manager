# fidesys_load_mamnager
Тестовое задание для Fidesys, для иллюстрации навыков работы с qt.

Задание:
Написать программу, которая позволит посмотреть приложенные нагрузки и ограничения в модели,
поменять их в случае необходимости и отправить на расчёт по нажатию кнопки.
в качестве рассматриваемых ГУ рассмотреть: - перемещения.
в качестве нагрузок: точечная сила, давление, распределённая нагрузка.

Подробности: 
Файл проекта Fidesys можно экспортировать в формате .fc который далее рассматривается как JsonObject
Через инструменты Qt Json создаётся копия с изменнёными параметрами и отправляется на расчёт.

Работает на C++, qt6.

