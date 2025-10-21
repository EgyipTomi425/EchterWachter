module;

#include <QCoreApplication>

export module qt;

inline int qt_argc = 0;
inline char** qt_argv = nullptr;
export inline QCoreApplication qt(qt_argc, qt_argv);