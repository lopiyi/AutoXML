#include <qmessagebox.h>
#include <QtWidgets/QApplication>
#include <Windows.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //QtWidgetsApplication w;
    //w.show();
    //return a.exec();
    HWND dnf_win = FindWindowA(NULL, (LPCSTR)"���³�����ʿ����������");
    RECT r1;
    GetWindowRect(dnf_win, &r1);
    QString mes = QString(u8"��ϷX���꣺%1\n��ϷY���꣺%2").arg(r1.left).arg(r1.top);
    QMessageBox::information(NULL, u8"��ʾ", mes);
}
