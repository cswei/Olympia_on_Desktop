#ifndef OlympiaPlatformMainthreadInvoker_h
#define OlympiaplatformMainthreadInvoker_h
#include <QObject>
#include <QtCore/QCoreApplication>
#include <QThread>
namespace Olympia {
    namespace Platform {
        typedef void(*CallBackFunc)(void);
        class MainThreadInvoker : public QObject {
            Q_OBJECT
        public:
            MainThreadInvoker();
            static void setCallBack(CallBackFunc callback);
        private:
            static CallBackFunc m_callback;
        private slots:
            void dispatch();
        };
    }
}
#endif