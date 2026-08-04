/****************************************************************************
** Meta object code from reading C++ file 'testmanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../testmanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'testmanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TestManager_t {
    QByteArrayData data[17];
    char stringdata0[169];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TestManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TestManager_t qt_meta_stringdata_TestManager = {
    {
QT_MOC_LITERAL(0, 0, 11), // "TestManager"
QT_MOC_LITERAL(1, 12, 12), // "stateChanged"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 5), // "State"
QT_MOC_LITERAL(4, 32, 8), // "newState"
QT_MOC_LITERAL(5, 41, 10), // "logMessage"
QT_MOC_LITERAL(6, 52, 3), // "msg"
QT_MOC_LITERAL(7, 56, 19), // "channelStateChanged"
QT_MOC_LITERAL(8, 76, 7), // "channel"
QT_MOC_LITERAL(9, 84, 5), // "state"
QT_MOC_LITERAL(10, 90, 16), // "channelCompleted"
QT_MOC_LITERAL(11, 107, 5), // "avgOd"
QT_MOC_LITERAL(12, 113, 13), // "concentration"
QT_MOC_LITERAL(13, 127, 14), // "globalProgress"
QT_MOC_LITERAL(14, 142, 7), // "current"
QT_MOC_LITERAL(15, 150, 5), // "total"
QT_MOC_LITERAL(16, 156, 12) // "goToNextStep"

    },
    "TestManager\0stateChanged\0\0State\0"
    "newState\0logMessage\0msg\0channelStateChanged\0"
    "channel\0state\0channelCompleted\0avgOd\0"
    "concentration\0globalProgress\0current\0"
    "total\0goToNextStep"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TestManager[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       5,    1,   47,    2, 0x06 /* Public */,
       7,    2,   50,    2, 0x06 /* Public */,
      10,    3,   55,    2, 0x06 /* Public */,
      13,    2,   62,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      16,    0,   67,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 3,    8,    9,
    QMetaType::Void, QMetaType::Int, QMetaType::Double, QMetaType::Double,    8,   11,   12,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   14,   15,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void TestManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TestManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->stateChanged((*reinterpret_cast< State(*)>(_a[1]))); break;
        case 1: _t->logMessage((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->channelStateChanged((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< State(*)>(_a[2]))); break;
        case 3: _t->channelCompleted((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 4: _t->globalProgress((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->goToNextStep(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TestManager::*)(State );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TestManager::stateChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (TestManager::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TestManager::logMessage)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (TestManager::*)(int , State );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TestManager::channelStateChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (TestManager::*)(int , double , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TestManager::channelCompleted)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (TestManager::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&TestManager::globalProgress)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TestManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_TestManager.data,
    qt_meta_data_TestManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TestManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TestManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TestManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int TestManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void TestManager::stateChanged(State _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void TestManager::logMessage(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void TestManager::channelStateChanged(int _t1, State _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void TestManager::channelCompleted(int _t1, double _t2, double _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void TestManager::globalProgress(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
