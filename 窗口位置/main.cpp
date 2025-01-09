#include <qmessagebox.h>
#include <QtWidgets/QApplication>
#include <Windows.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //QtWidgetsApplication w;
    //w.show();
    //return a.exec();
    HWND dnf_win = FindWindowA(NULL, (LPCSTR)"地下城与勇士：创新世纪");
    RECT r1;
    GetWindowRect(dnf_win, &r1);
    QString mes = QString(u8"游戏X坐标：%1\n游戏Y坐标：%2").arg(r1.left).arg(r1.top);
    QMessageBox::information(NULL, u8"提示", mes);
}
