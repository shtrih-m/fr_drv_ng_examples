#include "classic_interface.h"
#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <thread>

static void checkResult(int code)
{
    if (code != 0) {
        std::stringstream ss;
        ss << "error, bad return code: " << code;
        throw std::runtime_error(ss.str());
    }
}

static void executeAndHandleError(std::function<int()> func)
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

static void print2DBarcode(classic_interface* ci)
{
    ci->Set_BarCode(
        u8"это печать unicode строки в QR коде с выравниванием по разным сторонам чека");
    ci->Set_BlockNumber(0);
    ci->Set_BarcodeType(BC2D_QRCODE);
    ci->Set_BarcodeParameter1(0); //авто версия
    ci->Set_BarcodeParameter3(4); //размер точки
    ci->Set_BarcodeParameter5(3); //уровень коррекции ошибок 0...3=L,M,Q,H
    for (const auto& alignment :
        { TBarcodeAlignment::baCenter, TBarcodeAlignment::baLeft, TBarcodeAlignment::baRight }) {
        ci->Set_BarcodeAlignment(alignment);
        executeAndHandleError(std::bind(&classic_interface::LoadAndPrint2DBarcode, std::ref(ci)));
    }
}

/**
 * @brief The PasswordHolder class
 * держатель паролей, восстанавливает пред. пароль при выходе из области видимости
 */
class PasswordHolder {
    classic_interface* m_ci;
    uint32_t m_oldPassword;

public:
    PasswordHolder(const PasswordHolder&) = delete;
    PasswordHolder(const PasswordHolder&&) = delete;
    PasswordHolder& operator=(const PasswordHolder&) = delete;
    PasswordHolder& operator=(const PasswordHolder&&) = delete;
    PasswordHolder(classic_interface* ci, uint32_t tempPasword)
        : m_ci(ci)
        , m_oldPassword(ci->Get_Password())
    {
        m_ci->Set_Password(tempPasword);
    }
    ~PasswordHolder()
    {
        m_ci->Set_Password(m_oldPassword);
    }
};

static void prepareRecepit(classic_interface* ci)
{
    executeAndHandleError(
        std::bind(&classic_interface::GetECRStatus, std::ref(ci))); //получаем статус
    switch (ci->Get_ECRMode()) {
    case 3: {
        PasswordHolder ph(ci, ci->Get_SysAdminPassword()); // для снятия Z отчета необходимо
                                                           // воспользоваться паролем администратора
        executeAndHandleError(std::bind(&classic_interface::PrintReportWithCleaning,
            ci)); //снимаем Z отчет если смена больше 24 часов
    } break;
    case 4:
        executeAndHandleError(std::bind(&classic_interface::OpenSession,
            ci)); //если смена закрыта - открываем
        break;
    case 8:
        executeAndHandleError(std::bind(&classic_interface::SysAdminCancelCheck,
            ci)); // отменяем документ если открыт
        break;
    }
    executeAndHandleError(std::bind(&classic_interface::WaitForPrinting, std::ref(ci)));
}

static void print1Dbarcode(classic_interface* ci, const std::string& codeData)
{
    ci->Set_BarCode(codeData);
    ci->Set_LineNumber(10); //высота ШК в линиях
    ci->Set_BarWidth(2); //ширина вертикальной линии ШК
    executeAndHandleError(std::bind(&classic_interface::WaitForPrinting, std::ref(ci)));

    for (const auto& codeType : { BC1D_Code128A, BC1D_Code39, BC1D_EAN13 }) {
        ci->Set_BarcodeType(codeType); //тип
        for (const auto& alignment : { TBarcodeAlignment::baCenter, TBarcodeAlignment::baLeft,
                 TBarcodeAlignment::baRight }) {
            ci->Set_BarcodeAlignment(alignment); //выравнивание
            for (const auto& text_alignment : { BCT_None, BCT_Above, BCT_Below, BCT_Both }) {
                ci->Set_PrintBarcodeText(text_alignment); //печать текста ШК
                executeAndHandleError(
                    std::bind(&classic_interface::PrintBarcodeLine, std::ref(ci)));
                executeAndHandleError(std::bind(&classic_interface::WaitForPrinting, std::ref(ci)));
            }
        }
    }
}

/**
 * @brief cashierReceipt
 * @param ci
 * @param cancel отменить чек
 */
static void cashierReceipt(classic_interface* ci, bool cancel = false)
{
    prepareRecepit(ci);
    executeAndHandleError(
        std::bind(&classic_interface::OpenCheck, std::ref(ci))); //открываем чек с паролем кассира
    ci->Set_Quantity(1.0);
    ci->Set_Department(0);
    ci->Set_Price(10000);
    ci->Set_Tax1(0);
    ci->Set_Tax2(0);
    ci->Set_Tax3(0);
    ci->Set_Tax4(0);
    ci->Set_StringForPrinting(u8"Молоко");
    executeAndHandleError(std::bind(&classic_interface::Sale, std::ref(ci)));
    if (cancel) {
        executeAndHandleError(std::bind(&classic_interface::SysAdminCancelCheck, std::ref(ci)));
        return;
    }
    ci->Set_Summ1(100000);
    ci->Set_StringForPrinting(u8"строчка");
    executeAndHandleError(std::bind(&classic_interface::CloseCheck, std::ref(ci)));
    executeAndHandleError(std::bind(&classic_interface::WaitForPrinting, std::ref(ci)));
}
static void adminCancelReceipt(classic_interface* ci)
{
    cashierReceipt(ci, true);
}

int main(int argc, char* argv[])
{
    try {
        classic_interface ci;
        ci.Set_SysAdminPassword(30); //Пароль сист. администратора
        ci.Set_Password(1); //Пароль кассира(может совпадать с паролем администратора)
        ci.Set_AutoEoD(true); //Включаем обмен с ОФД средствами драйвера
        if (argc > 1) {
            //можно передать URI в качестве аргумента
            ci.Set_ConnectionURI(argv[1]);
        } else {
            ci.Set_ConnectionURI(
                "tcp://192.168.137.111:7778?timeout=3000&bytetimeout=1500&protocol=v1");
        }
        checkResult(ci.Connect()); //соединяемся
        cashierReceipt(&ci); //чек от кассира 1
        adminCancelReceipt(&ci); //открываем чек от имени кассира 1, отмена от администратора
        ci.Set_Password(2);
        cashierReceipt(&ci); //чек от кассира 2
        print1Dbarcode(&ci, "123456789"); //пример одномерных ШК
        print2DBarcode(&ci); //пример QR кода
        ci.Set_StringQuantity(10); //кол-во строк промотки
        ci.Set_UseReceiptRibbon(true); //использовать чековую лента
        ci.Set_CutType(false); //полная отрезка
        executeAndHandleError(std::bind(&classic_interface::FeedDocument, std::ref(ci))); //промотка
        executeAndHandleError(std::bind(&classic_interface::CutCheck, std::ref(ci))); //отрезка

        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
    }
    return EXIT_FAILURE;
}
