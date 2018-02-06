#include "classic_interface.h"
#include <exception>
#include <iostream>
#include <sstream>

using namespace std;
static void checkResult(int code)
{
    if (code) {
        stringstream ss;
        ss << "error, bad return code: " << code;
        throw runtime_error(ss.str());
    }
}

int main(int argc, char* argv[])
{
    try {
        classic_interface ci;
        ci.Set_Password(30);
        if (argc > 1) {
            //можно передать URI в качестве аргумента
            ci.Set_ConnectionURI(argv[1]);
        } else {
            ci.Set_ConnectionURI("tcp://192.168.137.111:7778?timeout=1000&protocol=v1");
        }
        checkResult(ci.Connect()); //соединяемся
        checkResult(ci.GetECRStatus()); //получаем статус
        switch (ci.Get_ECRMode()) {
        case 3:
            checkResult(ci.PrintReportWithCleaning()); //снимаем Z отчет если смена больше 24 часов
            break;
        case 4:
            checkResult(ci.OpenSession()); //если смена закрыта - открываем
            break;
        case 8:
            checkResult(ci.CancelCheck()); // отменяем документ если открыт
            break;
        }
        checkResult(ci.OpenCheck()); //открываем чек
        ci.Set_Password(30);
        ci.Set_Quantity(1.0);
        ci.Set_Department(1);
        ci.Set_Price(10000);
        ci.Set_Tax1(0);
        ci.Set_Tax2(0);
        ci.Set_Tax3(0);
        ci.Set_Tax4(0);
        ci.Set_StringForPrinting(u8"Молоко");
        checkResult(ci.Sale());
        ci.Set_Summ1(10000);
        ci.Set_StringForPrinting(u8"строчка");
        checkResult(ci.CloseCheck());
        return EXIT_SUCCESS;
    } catch (const exception& e) {
        cerr << e.what() << endl;
    } catch (...) {
        cerr << "unknown error" << endl;
    }
    return EXIT_FAILURE;
}
