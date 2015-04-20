/****************************************************************************
** Meta object code from reading C++ file 'euRun.hh'
**
** Created: Mon Apr 20 10:32:19 2015
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../gui/include/euRun.hh"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'euRun.hh' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_RunControlGUI[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      17,   15,   14,   14, 0x05,
      55,   48,   14,   14, 0x05,
      77,   48,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      97,   91,   14,   14, 0x08,
     115,   14,   14,   14, 0x08,
     141,   14,   14,   14, 0x08,
     169,  164,   14,   14, 0x08,
     195,   14,   14,   14, 0x28,
     217,   14,   14,   14, 0x08,
     238,   14,   14,   14, 0x08,
     258,   14,   14,   14, 0x08,
     277,  266,   14,   14, 0x08,
     307,   48,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_RunControlGUI[] = {
    "RunControlGUI\0\0,\0StatusChanged(QString,QString)\0"
    "status\0btnLogSetStatus(bool)\0SetState(int)\0"
    "state\0SetStateSlot(int)\0"
    "on_btnTerminate_clicked()\0"
    "on_btnConfig_clicked()\0cont\0"
    "on_btnStart_clicked(bool)\0"
    "on_btnStart_clicked()\0on_btnStop_clicked()\0"
    "on_btnLog_clicked()\0timer()\0name,value\0"
    "ChangeStatus(QString,QString)\0"
    "btnLogSetStatusSlot(bool)\0"
};

const QMetaObject RunControlGUI::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_RunControlGUI,
      qt_meta_data_RunControlGUI, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &RunControlGUI::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *RunControlGUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *RunControlGUI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_RunControlGUI))
        return static_cast<void*>(const_cast< RunControlGUI*>(this));
    if (!strcmp(_clname, "Ui::wndRun"))
        return static_cast< Ui::wndRun*>(const_cast< RunControlGUI*>(this));
    if (!strcmp(_clname, "eudaq::RunControl"))
        return static_cast< eudaq::RunControl*>(const_cast< RunControlGUI*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int RunControlGUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: StatusChanged((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 1: btnLogSetStatus((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: SetState((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: SetStateSlot((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: on_btnTerminate_clicked(); break;
        case 5: on_btnConfig_clicked(); break;
        case 6: on_btnStart_clicked((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: on_btnStart_clicked(); break;
        case 8: on_btnStop_clicked(); break;
        case 9: on_btnLog_clicked(); break;
        case 10: timer(); break;
        case 11: ChangeStatus((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 12: btnLogSetStatusSlot((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void RunControlGUI::StatusChanged(const QString & _t1, const QString & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void RunControlGUI::btnLogSetStatus(bool _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void RunControlGUI::SetState(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
