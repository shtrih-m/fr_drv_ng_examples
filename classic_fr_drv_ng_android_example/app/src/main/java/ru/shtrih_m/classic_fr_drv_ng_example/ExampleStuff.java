package ru.shtrih_m.classic_fr_drv_ng_example;

import java.util.Vector;
import java.util.concurrent.Callable;

import ru.shtrih_m.fr_drv_ng.classic_interface.classic_interface;

public class ExampleStuff {
    public static void checkResult(long ret) {
        if (ret != 0) {
            throw new RuntimeException("error, bad return code: " + ret);
        }
    }

    /**
     * @param func
     * @brief executeAndHandleError выполнить функцию и переповторить, если принтер занят печатью
     */
    private static void executeAndHandleError(Callable<Integer> func) {
        for (; ; ) {
            int ret = -8;
            try {
                ret = func.call();
            } catch (Exception e) {
                e.printStackTrace();
            }
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
     * @param ci объект ci для которого необходимо выполнить
     * @return 0
     * @brief isCashcore получить параметр кассы, реализация - кассовое ядро или аппаратный принтер
     */
    static boolean isCashcore(final classic_interface ci) {
        ci.Set_ModelParamNumber(classic_interface.DevicePropertiesEnumeration.DPE_f23_cashcore.swigValue());
        checkResult(ci.ReadModelParamValue());
        return ci.Get_ModelParamValue() == 1;
    }


    public static void prepareReceipt(final classic_interface ci) {
        if (ci.Get_ECRMode() == classic_interface.PrinterMode.PM_SessionOpenOver24h.swigValue()) {
            long oldPassword = ci.Get_Password();
            ci.Set_Password(ci.Get_SysAdminPassword());
            executeAndHandleError(new Callable<Integer>() {
                @Override
                public Integer call() {
                    return ci.PrintReportWithCleaning();//снимаем Z отчет если смена больше 24 часов
                }
            });
            ci.Set_Password(oldPassword);
        } else if (ci.Get_ECRMode() == classic_interface.PrinterMode.PM_SessionClosed.swigValue()) {
            executeAndHandleError(new Callable<Integer>() {
                @Override
                public Integer call() {
                    return ci.OpenSession(); //если смена закрыта - открываем
                }
            });
        } else if (ci.Get_ECRMode() == classic_interface.PrinterMode.PM_OpenedDocument.swigValue()) {
            executeAndHandleError(new Callable<Integer>() {
                @Override
                public Integer call() {
                    return ci.SysAdminCancelCheck(); //если смена закрыта - открываем
                }
            });
        }
        executeAndHandleError(new Callable<Integer>() {
            @Override
            public Integer call() {
                return ci.WaitForPrinting(); //если смена закрыта - открываем
            }
        });
    }


    /**
     * @param ci
     * @brief fsOperationReceipt продвинутыми командами ФН
     * Передача КТН (тег 1162 для табачной продукции)
     */
    public static void fsOperationReceipt(final classic_interface ci) {
        class Item {
            long price;
            double quantity;
            long total; // -1 - рассчитывает касса, иначе сами
            String name;
            boolean bGTINexample; // пример товара с маркировкой

        }
        Vector<Item> items = new Vector<>();
        {
            Item i1 = new Item();
            i1.price = 12300;
            i1.quantity = 1.009456;
            i1.total = -1;
            i1.name = "Традиционное молоко";
            i1.bGTINexample = false;
//            items.add(i1);
            Item i2 = new Item();
            i2.price = 4440;
            i2.quantity = 4;
            i2.total = 17761;//сумма с *ошибкой* на копейку
            i2.name = "Товар";
            i2.bGTINexample = false;

            Item i3 = new Item();
            i3.price = 5000;
            i3.quantity = 1;
            i3.total = 5000;
            i3.name = "Сигареты Прима";
            i3.bGTINexample = true;
            items.add(i1);
            items.add(i2);
            items.add(i3);
        }


        prepareReceipt(ci);
        long sum = 0;
        ci.Set_CheckType(0); //продажа
        executeAndHandleError(new Callable<Integer>() {
            @Override
            public Integer call() {
                return ci.OpenCheck();
            }
        });

        for (Item item : items) {
            ci.Set_CheckType(1); //приход
            ci.Set_Quantity(item.quantity);
            ci.Set_Price(item.price);
            sum += item.price * item.quantity;

            ci.Set_Summ1Enabled(!(item.total == -1)); //рассчитывает касса
            ci.Set_Summ1(item.total);
            ci.Set_TaxValueEnabled(false);
            ci.Set_Tax1(1); //НДС 18%
            ci.Set_Department(1);
            ci.Set_PaymentTypeSign(4); //полный рассчет
            ci.Set_PaymentItemSign(1); //товар
            ci.Set_StringForPrinting(item.name);
            if (item.bGTINexample) {
                boolean cashcore = isCashcore(ci); // узнаем на кассовом ядре мы работаем или нет
                ci.Set_MarkingType(5); //Табачные изделия
                ci.Set_GTIN("12345678901234"); // 14-ти значное число
                ci.Set_SerialNumber("987654321001234567890123");
                if (cashcore) {
                    //посылать тег, привязанный к операции на cashcore(Кассовое Ядро) нужно ДО операции
                    executeAndHandleError(new Callable<Integer>() {
                        @Override
                        public Integer call() {
                            return ci.FNSendItemCodeData();
                        }
                    });
                }
                executeAndHandleError(new Callable<Integer>() {
                    @Override
                    public Integer call() {
                        return ci.FNOperation();
                    }
                });
                if (!cashcore) {
                    //иначе ПОСЛЕ
                    executeAndHandleError(new Callable<Integer>() {
                        @Override
                        public Integer call() {
                            return ci.FNSendItemCodeData();
                        }
                    });
                }
            } else {
                executeAndHandleError(new Callable<Integer>() {
                    @Override
                    public Integer call() {
                        return ci.FNOperation();
                    }
                });
            }
        }
        long cash_sum = 15000; //частично оплатим начлиными
        long electro_sum = sum - cash_sum; //остальное электронными
        ci.Set_Summ1(cash_sum); // Наличные
        ci.Set_Summ2(electro_sum); //Электронными
        ci.Set_Summ3(0);
        ci.Set_Summ4(0);
        ci.Set_Summ5(0);
        ci.Set_Summ6(0);
        ci.Set_Summ7(0);
        ci.Set_Summ8(0);
        ci.Set_Summ9(0);
        ci.Set_Summ10(0);
        ci.Set_Summ11(0);
        ci.Set_Summ12(0);
        ci.Set_Summ13(0);
        ci.Set_Summ14(0);
        ci.Set_Summ15(0);
        ci.Set_Summ16(0);
        ci.Set_RoundingSumm(99); // Сумма округления
        ci.Set_TaxValue1(0); // Налоги мы не считаем
        ci.Set_TaxValue2(0);
        ci.Set_TaxValue3(0);
        ci.Set_TaxValue4(0);
        ci.Set_TaxValue5(0);
        ci.Set_TaxValue6(0);
        ci.Set_TaxType(1); // Основная система налогообложения
        ci.Set_StringForPrinting("");
        executeAndHandleError(new Callable<Integer>() {
            @Override
            public Integer call() {
                return ci.FNCloseCheckEx();
            }
        });
        executeAndHandleError(new Callable<Integer>() {
            @Override
            public Integer call() {
                return ci.WaitForPrinting(); //если смена закрыта - открываем
            }
        });
    }
}
