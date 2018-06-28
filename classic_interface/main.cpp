#include "classic_interface.h"
#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <thread>

/**
 * @brief checkResult проверяет код ошибки и бросает исключение если таковая
 * @param code
 */
static void checkResult(int code)
{
    if (code != 0) {
        std::stringstream ss;
        ss << "error, bad return code: " << code;
        throw std::runtime_error(ss.str());
    }
}

/**
 * @brief executeAndHandleError выполнить функцию и переповторить, если принтер занят печатью
 * @param func
 */
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

/**
 * @brief print2DBarcode печать QR кода
 * @param ci
 */
static void print2DBarcode(classic_interface* ci)
{
    ci->Set_BarCode(
        u8"это печать unicode строки в QR коде с выравниванием по разным сторонам чека");
    ci->Set_BlockNumber(0);
    ci->Set_BarcodeType(classic_interface::BC2D_QRCODE);
    ci->Set_BarcodeParameter1(0); //авто версия
    ci->Set_BarcodeParameter3(4); //размер точки
    ci->Set_BarcodeParameter5(3); //уровень коррекции ошибок 0...3=L,M,Q,H
    for (const auto& alignment : { classic_interface::TBarcodeAlignment::baCenter,
             classic_interface::TBarcodeAlignment::baLeft,
             classic_interface::TBarcodeAlignment::baRight }) {
        ci->Set_BarcodeAlignment(alignment);
        executeAndHandleError(std::bind(&classic_interface::LoadAndPrint2DBarcode, ci));
    }
}

/**
 * @brief The PasswordHolder class
 * держатель паролей, восстанавливает пред. пароль при выходе из области видимости
 */
class PasswordHolder {
public:
    enum PasswordType {
        PT_User, //пользователь/кассир
        PT_Admin, //администратор
        PT_SC //пароль цто
    };

private:
    classic_interface* m_ci;
    uint32_t m_oldPassword;
    PasswordType m_passwordType;
    static uint32_t oldPasswordByType(classic_interface* ci, PasswordType passwordType)
    {
        switch (passwordType) {
        case PasswordHolder::PT_Admin:
            return ci->Get_SysAdminPassword();
        case PasswordHolder::PT_SC:
            return ci->Get_SCPassword();
        default:
            return ci->Get_Password();
        }
    }

public:
    PasswordHolder(const PasswordHolder&) = delete;
    PasswordHolder(const PasswordHolder&&) = delete;
    PasswordHolder& operator=(const PasswordHolder&) = delete;
    PasswordHolder& operator=(const PasswordHolder&&) = delete;
    PasswordHolder(classic_interface* ci, uint32_t tempPasword, PasswordType passwordType = PT_User)
        : m_ci(ci)
        , m_oldPassword(oldPasswordByType(ci, passwordType))
        , m_passwordType(passwordType)
    {
        switch (m_passwordType) {
        case PasswordHolder::PT_User:
            m_ci->Set_Password(tempPasword);
            break;
        case PasswordHolder::PT_Admin:
            m_ci->Set_SysAdminPassword(tempPasword);
            break;
        case PasswordHolder::PT_SC:
            m_ci->Set_SCPassword(tempPasword);
            break;
        }
    }
    ~PasswordHolder()
    {
        switch (m_passwordType) {
        case PasswordHolder::PT_User:
            m_ci->Set_Password(m_oldPassword);
            break;
        case PasswordHolder::PT_Admin:
            m_ci->Set_SysAdminPassword(m_oldPassword);
            break;
        case PasswordHolder::PT_SC:
            m_ci->Set_SCPassword(m_oldPassword);
            break;
        }
    }
};

static void prepareRecepit(classic_interface* ci)
{
    executeAndHandleError(std::bind(&classic_interface::WaitForPrinting, ci));
    executeAndHandleError(std::bind(&classic_interface::GetECRStatus, ci)); //получаем статус
    switch (ci->Get_ECRMode()) {
    case 3: {
        {
            PasswordHolder ph(
                ci, ci->Get_SysAdminPassword()); // для снятия Z отчета необходимо
                                                 // воспользоваться паролем администратора
            executeAndHandleError(std::bind(&classic_interface::PrintReportWithCleaning,
                ci)); //снимаем Z отчет если смена больше 24 часов
            executeAndHandleError(std::bind(&classic_interface::WaitForPrinting,
                ci)); //ждём пока отчет распечатается и открываем смену
        }
        executeAndHandleError(std::bind(&classic_interface::OpenSession,
            ci)); //смена закрыта - открываем
    }; break;
    case 4:
        executeAndHandleError(std::bind(&classic_interface::OpenSession,
            ci)); //если смена закрыта - открываем
        break;
    case 8:
        executeAndHandleError(std::bind(&classic_interface::SysAdminCancelCheck,
            ci)); // отменяем документ если открыт
        break;
    }
    executeAndHandleError(std::bind(&classic_interface::WaitForPrinting, ci));
}
static void exchangeBytes(classic_interface* ci)
{
    ci->Set_BinaryConversion(classic_interface::TBinaryConversion::BINARY_CONVERSION_HEX);
    ci->Set_TransferBytes("FC");
    executeAndHandleError(std::bind(&classic_interface::ExchangeBytes, ci));
}

/**
 * @brief print1Dbarcode одномерные штрих-коды
 * @param ci
 * @param codeData
 */
static void print1Dbarcode(classic_interface* ci, const std::string& codeData)
{
    ci->Set_BarCode(codeData);
    ci->Set_LineNumber(50); //высота ШК в линиях
    ci->Set_BarWidth(2); //ширина вертикальной линии ШК
    executeAndHandleError(std::bind(&classic_interface::WaitForPrinting, ci));

    for (const auto& codeType : { classic_interface::BC1D_Code128A, classic_interface::BC1D_Code39,
             classic_interface::BC1D_EAN13 }) {
        ci->Set_BarcodeType(codeType); //тип
        for (const auto& alignment : { classic_interface::TBarcodeAlignment::baCenter,
                 classic_interface::TBarcodeAlignment::baLeft,
                 classic_interface::TBarcodeAlignment::baRight }) {
            ci->Set_BarcodeAlignment(alignment); //выравнивание
            for (const auto& text_alignment :
                { classic_interface::BCT_None, classic_interface::BCT_Above,
                    classic_interface::BCT_Below, classic_interface::BCT_Both }) {
                ci->Set_PrintBarcodeText(text_alignment); //печать текста ШК
                executeAndHandleError(std::bind(&classic_interface::PrintBarcodeLine, ci));
                executeAndHandleError(std::bind(&classic_interface::WaitForPrinting, ci));
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
    ci->Set_CheckType(0); //продажа
    executeAndHandleError(
        std::bind(&classic_interface::OpenCheck, ci)); //открываем чек с паролем кассира
    ci->Set_Quantity(1.0);
    ci->Set_Department(0);
    ci->Set_Price(10000);
    ci->Set_Tax1(0);
    ci->Set_Tax2(0);
    ci->Set_Tax3(0);
    ci->Set_Tax4(0);
    ci->Set_StringForPrinting(u8"Молоко");
    executeAndHandleError(std::bind(&classic_interface::Sale, ci));
    if (cancel) {
        executeAndHandleError(std::bind(&classic_interface::SysAdminCancelCheck, ci));
        return;
    }
    ci->Set_Summ1(100000);
    ci->Set_Summ2(0);
    ci->Set_Summ3(0);
    ci->Set_Summ4(0);
    ci->Set_Summ5(0);
    ci->Set_Summ6(0);
    ci->Set_Summ7(0);
    ci->Set_Summ8(0);
    ci->Set_Summ9(0);
    ci->Set_Summ10(0);
    ci->Set_Summ11(0);
    ci->Set_Summ12(0);
    ci->Set_Summ13(0);
    ci->Set_Summ14(0);
    ci->Set_Summ15(0);
    ci->Set_Summ16(0);
    ci->Set_StringForPrinting(u8"строчка");
    executeAndHandleError(std::bind(&classic_interface::CloseCheck, ci));
    executeAndHandleError(std::bind(&classic_interface::WaitForPrinting, ci));
}

/**
 * @brief fsOperationReceipt продвинутыми командами ФН
 * Передача КТН (тег 1162 для табачной продукции)
 * @param ci
 */
static void fsOperationReceipt(classic_interface* ci)
{
    prepareRecepit(ci);
    ci->Set_CheckType(0); //продажа
    executeAndHandleError(std::bind(&classic_interface::OpenCheck, ci));
    ci->Set_CheckType(1); //приход
    ci->Set_Quantity(1.009456);
    ci->Set_Price(12300);
    ci->Set_Summ1Enabled(false); //рассчитывает касса
    ci->Set_TaxValueEnabled(false);
    ci->Set_Tax1(1); //НДС 18%
    ci->Set_Department(1);
    ci->Set_PaymentTypeSign(4); //полный рассчет
    ci->Set_PaymentItemSign(1); //товар
    ci->Set_StringForPrinting(u8"Традиционное молоко");
    executeAndHandleError(std::bind(&classic_interface::FNOperation, ci));
    ci->Set_CheckType(1); //приход
    ci->Set_Quantity(4);
    ci->Set_Price(4440);
    ci->Set_Summ1Enabled(true); //рассчитываем сами
    ci->Set_Summ1(17761); //*ошибаемся* на копейку
    ci->Set_TaxValueEnabled(false);
    ci->Set_Tax1(1); //НДС 18%
    ci->Set_Department(1);
    ci->Set_PaymentTypeSign(4); //полный рассчет
    ci->Set_PaymentItemSign(1); //товар
    ci->Set_StringForPrinting(u8"Товар");
    executeAndHandleError(std::bind(&classic_interface::FNOperation, ci));
    ci->Set_CheckType(1); //приход
    ci->Set_Quantity(1);
    ci->Set_Price(5000);
    ci->Set_Summ1Enabled(true); //рассчитываем сами
    ci->Set_Summ1(5000);
    ci->Set_TaxValueEnabled(false);
    ci->Set_Tax1(1); //НДС 18%
    ci->Set_Department(1);
    ci->Set_PaymentTypeSign(4); //полный рассчет
    ci->Set_PaymentItemSign(1); //товар
    ci->Set_StringForPrinting(u8"Сигареты Прима");

    ci->Set_MarkingType(5); //Табачные изделия
    ci->Set_GTIN("12345678901234"); // 14-ти значное число
    ci->Set_SerialNumber("987654321001234567890123");
    ci->Set_ModelParamNumber(classic_interface::DPE_f23_cashcore);
    checkResult(ci->ReadModelParamValue());
    auto isCashcore = ci->Get_ModelParamValue();
    if (isCashcore) {
        //посылать тег, привязанный к операции на cashcore(Кассовое Ядро) нужно ДО операции
        executeAndHandleError(std::bind(&classic_interface::FNSendItemCodeData, ci));
    }
    executeAndHandleError(std::bind(&classic_interface::FNOperation, ci));
    if (!isCashcore) {
        //иначе ПОСЛЕ
        executeAndHandleError(std::bind(&classic_interface::FNSendItemCodeData, ci));
    }
    ci->Set_Summ1(17761 + 5000); // Наличные
    ci->Set_Summ2(static_cast<int64_t>(12300 * 1.009456)); //Электронными
    ci->Set_Summ3(0);
    ci->Set_Summ4(0);
    ci->Set_Summ5(0);
    ci->Set_Summ6(0);
    ci->Set_Summ7(0);
    ci->Set_Summ8(0);
    ci->Set_Summ9(0);
    ci->Set_Summ10(0);
    ci->Set_Summ11(0);
    ci->Set_Summ12(0);
    ci->Set_Summ13(0);
    ci->Set_Summ14(0);
    ci->Set_Summ15(0);
    ci->Set_Summ16(0);
    ci->Set_RoundingSumm(99); // Сумма округления
    ci->Set_TaxValue1(0); // Налоги мы не считаем
    ci->Set_TaxValue2(0);
    ci->Set_TaxValue3(0);
    ci->Set_TaxValue4(0);
    ci->Set_TaxValue5(0);
    ci->Set_TaxValue6(0);
    ci->Set_TaxType(1); // Основная система налогообложения
    ci->Set_StringForPrinting("");
    executeAndHandleError(std::bind(&classic_interface::FNCloseCheckEx, ci));
}

static void adminCancelReceipt(classic_interface* ci)
{
    cashierReceipt(ci, true);
}

/**
 * @brief writeServiceTable пример записи служебной таблицы с паролем ЦТО
 */
static void writeServiceTable(classic_interface* ci)
{
    prepareRecepit(ci);
    auto valueToWrite = 30;
    {
        ci->Set_TableNumber(10);
        ci->Set_RowNumber(1);
        ci->Set_FieldNumber(10);
        executeAndHandleError(std::bind(&classic_interface::ReadTable, ci));
        //получили, закешировали структуру поля из служебной таблицы. Можно поменять пароль
        //администратора и изменить содержимое через WriteTable
        ci->Set_ValueOfFieldInteger(valueToWrite);
        {
            PasswordHolder ph(ci, ci->Get_SCPassword(), PasswordHolder::PT_Admin);
            executeAndHandleError(std::bind(&classic_interface::WriteTable, ci));
        }
    }
}
static void printLineSwaps(classic_interface* ci)
{
    prepareRecepit(ci);
    std::string line{ 1, 2, 4, 8, 16, 32, 64, static_cast<char>(128), 64, 32, 16, 8, 4, 2, 1 };
    ci->Set_LineData(line);
    ci->Set_LineNumber(50);
    for (const auto& swapMode : { classic_interface::SBM_Swap, classic_interface::SBM_NoSwap,
             classic_interface::SBM_Prop, classic_interface::SBM_Model }) {
        ci->Set_SwapBytesMode(swapMode);
        executeAndHandleError(std::bind(&classic_interface::PrintLine, ci));
        executeAndHandleError(std::bind(&classic_interface::WaitForPrinting, ci));
    }
}

static void printBarcodeLineSwaps(classic_interface* ci)
{
    prepareRecepit(ci);
    ci->Set_DelayedPrint(true);
    for (const auto& swapMode : { classic_interface::SBM_Swap, classic_interface::SBM_NoSwap,
             classic_interface::SBM_Prop, classic_interface::SBM_Model }) {
        {
            std::stringstream ss;
            ss << "mode: " << swapMode;
            ci->Set_BarCode(ss.str());
        }
        ci->Set_LineNumber(50);
        ci->Set_SwapBytesMode(swapMode);
        ci->Set_BarWidth(2);
        ci->Set_BarcodeType(classic_interface::BC1D_Code128A);
        ci->Set_BarcodeAlignment(classic_interface::TBarcodeAlignment::baCenter);
        ci->Set_PrintBarcodeText(classic_interface::BCT_Below);
        executeAndHandleError(std::bind(&classic_interface::PrintBarcodeLine, ci));
        executeAndHandleError(std::bind(&classic_interface::WaitForPrinting, ci));
    }
}

static void printBasicLines(classic_interface* ci)
{
    prepareRecepit(ci);

    ci->Set_UseReceiptRibbon(true); //чековая лента
    ci->Set_StringForPrinting("строчка");
    executeAndHandleError(std::bind(&classic_interface::PrintString, ci));
    ci->Set_StringForPrinting(u8"Мой дядя самых честных правил,\n"
                              "Когда не в шутку занемог,\n"
                              "Он уважать себя заставил\n"
                              "И лучше выдумать не мог.\n"
                              "Его пример другим наука;\n"
                              "Но, боже мой, какая скука\n"
                              "С больным сидеть и день и ночь,\n"
                              "Не отходя ни шагу прочь!\n"
                              "Какое низкое коварство\n"
                              "Полуживого забавлять,\n"
                              "Ему подушки поправлять,\n"
                              "Печально подносить лекарство,\n"
                              "Вздыхать и думать про себя:\n"
                              "Когда же черт возьмет тебя!\n");
    ci->Set_CarryStrings(true);
    executeAndHandleError(std::bind(&classic_interface::PrintString, ci));
    ci->Set_FontType(1);
    {
        PasswordHolder ph(ci, ci->Get_SysAdminPassword());
        executeAndHandleError(std::bind(&classic_interface::GetFontMetrics, ci));
    }
    for (auto i = 1; i <= ci->Get_FontCount(); i++) {
        ci->Set_FontType(i);
        executeAndHandleError(std::bind(&classic_interface::PrintStringWithFont, ci));
    }
}

int main(int argc, char* argv[])
{
    try {
        //        classic_interface::setLogCallback([](const std::string& logmsg) { std::cerr <<
        //        logmsg; });
        classic_interface ci;
        //        ci.setPropertyChangedCallback([](const std::string& property) {
        //            std::cout << "property modified: " << property << std::endl;
        //        });
        ci.Set_SCPassword(0); //Пароль ЦТО, нужен для установки нового пароля ЦТО + можно позже
                              //воспользоваться для записи служебных таблиц(им необходим пароль ЦТО)
        ci.Set_SysAdminPassword(30); //Пароль сист. администратора
        ci.Set_Password(1); //Пароль кассира(может совпадать с паролем администратора)
        //        ci.Set_AutoEoD(true); //Включаем обмен с ОФД средствами драйвера
        if (argc > 1) {
            //можно передать URI в качестве аргумента
            ci.Set_ConnectionURI(argv[1]);
        } else {
            ci.Set_ConnectionURI(
                "tcp://192.168.137.111:7778?timeout=3000&bytetimeout=1500&protocol=v1");
        }
        checkResult(ci.Connect()); //соединяемся
        printLineSwaps(&ci);
        printBarcodeLineSwaps(&ci);
        printBasicLines(&ci);
        fsOperationReceipt(&ci);
        //        writeServiceTable(&ci); // пример записи сервисной таблицы
        exchangeBytes(&ci); //посылка произвольных данных
        cashierReceipt(&ci); //чек от кассира 1
        adminCancelReceipt(&ci); //открываем чек от имени кассира 1, отмена от администратора
        ci.Set_Password(2);
        cashierReceipt(&ci); //чек от кассира 2
        print1Dbarcode(&ci, "123456789"); //пример одномерных ШК
        print2DBarcode(&ci); //пример QR кода
        ci.Set_StringQuantity(10); //кол-во строк промотки
        ci.Set_UseReceiptRibbon(true); //использовать чековую лента
        ci.Set_CutType(false); //полная отрезка
        executeAndHandleError(std::bind(&classic_interface::FeedDocument, &ci)); //промотка
        executeAndHandleError(std::bind(&classic_interface::CutCheck, &ci)); //отрезка

        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
    }
    return EXIT_FAILURE;
}
