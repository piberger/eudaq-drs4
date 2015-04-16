/****************************************************************************
** Meta object code from reading C++ file 'euProd.hh'
**
** Created: Thu Apr 16 15:43:43 2015
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../gui/include/euProd.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'euProd.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ProducerGUI[] = {

 // content:
       4,       // revision
       0,       // classname
       1,   14, // classinfo
       1,   16, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // classinfo: key, value
      25,   12,

 // slots: signature, parameters, type, tag, flags
      33,   32,   32,   32, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ProducerGUI[] = {
    "ProducerGUI\0Emlyn Corrin\0Author\0\0"
    "on_btnTrigger_clicked()\0"
};

const QMetaObject ProducerGUI::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_ProducerGUI,
      qt_meta_data_ProducerGUI, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ProducerGUI::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ProducerGUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ProducerGUI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ProducerGUI))
        return static_cast<void*>(const_cast< ProducerGUI*>(this));
    if (!strcmp(_clname, "Ui::wndProd"))
        return static_cast< Ui::wndProd*>(const_cast< ProducerGUI*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int ProducerGUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: on_btnTrigger_clicked(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
