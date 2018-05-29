#include "classic_interface.h"
#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <thread>

using namespace std;

static void checkResult(int code)
{
    if (code) {
        stringstream ss;
        ss << "error, bad return code: " << code;
        throw runtime_error(ss.str());
    }
}

static void executeAndHandleError(function<int()> func)
{
    for (;;) {
        auto ret = func();
        switch (ret) {
        case 0x50: //Переповтор при ошибке "идёт печать предыдущей команды"
            continue;
        default:
            checkResult(ret);
            return;
        }
    }
}

static void print2DBarcode(classic_interface& ci)
{
    ci.Set_BarCode(u8"это печать unicode строки в QR коде с выравниванием по разным сторонам чека");
    ci.Set_BlockNumber(0);
    ci.Set_BarcodeType(BC2D_QRCODE);
    ci.Set_BarcodeParameter1(0); //авто версия
    ci.Set_BarcodeParameter3(4); //размер точки
    ci.Set_BarcodeParameter5(3); //уровень коррекции ошибок 0...3=L,M,Q,H
    for (const auto& alignment :
        { TBarcodeAlignment::baCenter, TBarcodeAlignment::baLeft, TBarcodeAlignment::baRight }) {
        ci.Set_BarcodeAlignment(alignment);
        executeAndHandleError(std::bind(&classic_interface::LoadAndPrint2DBarcode, &ci));
    }
}

static void printExampleReceipt(classic_interface& ci)
{
    executeAndHandleError(std::bind(&classic_interface::GetECRStatus, &ci)); //получаем статус
    switch (ci.Get_ECRMode()) {
    case 3:
        executeAndHandleError(std::bind(&classic_interface::PrintReportWithCleaning,
            &ci)); //снимаем Z отчет если смена больше 24 часов
        break;
    case 4:
        executeAndHandleError(std::bind(&classic_interface::OpenSession,
            &ci)); //если смена закрыта - открываем
        break;
    case 8:
        executeAndHandleError(std::bind(&classic_interface::CancelCheck,
            &ci)); // отменяем документ если открыт
        break;
    }

    executeAndHandleError(std::bind(&classic_interface::OpenCheck, &ci)); //открываем чек
    ci.Set_Password(30);
    ci.Set_Quantity(1.0);
    ci.Set_Department(1);
    ci.Set_Price(10000);
    ci.Set_Tax1(0);
    ci.Set_Tax2(0);
    ci.Set_Tax3(0);
    ci.Set_Tax4(0);
    ci.Set_StringForPrinting(u8"Молоко");
    executeAndHandleError(std::bind(&classic_interface::Sale, &ci));
    ci.Set_Summ1(10000);
    ci.Set_StringForPrinting(u8"строчка");
    executeAndHandleError(std::bind(&classic_interface::CloseCheck, &ci));
}

static void print1Dbarcode(classic_interface& ci, const std::string& codeData)
{
    ci.Set_BarCode(codeData);
    ci.Set_LineNumber(50); //высота ШК в линиях
    ci.Set_BarWidth(2); //ширина вертикальной линии ШК
    for (const auto& codeType : { BC1D_Code128A, BC1D_Code39, BC1D_EAN13 }) {
        ci.Set_BarcodeType(codeType); //тип
        for (const auto& alignment : { TBarcodeAlignment::baCenter, TBarcodeAlignment::baLeft,
                 TBarcodeAlignment::baRight }) {
            ci.Set_BarcodeAlignment(alignment); //выравнивание
            for (const auto& text_alignment : { BCT_None, BCT_Above, BCT_Below, BCT_Both }) {
                ci.Set_PrintBarcodeText(text_alignment); //печать текста ШК
                executeAndHandleError(std::bind(&classic_interface::PrintBarcodeLine, &ci));
            }
        }
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
            ci.Set_ConnectionURI(
                "tcp://192.168.137.111:7778?timeout=3000&bytetimeout=1500&protocol=v1");
        }
        checkResult(ci.Connect()); //соединяемся
        printExampleReceipt(ci); //пример чека
        print1Dbarcode(ci, "123456789"); //пример одномерных ШК
        print2DBarcode(ci); //пример QR кода
        executeAndHandleError(std::bind(&classic_interface::FinishDocument, &ci));

        return EXIT_SUCCESS;
    } catch (const exception& e) {
        cerr << e.what() << endl;
    } catch (...) {
        cerr << "unknown error" << endl;
    }
    return EXIT_FAILURE;
}
