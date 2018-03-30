package ru.shtrih_m.classic_fr_drv_ng_android_example;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

import libcore.io.Libcore;
import ru.shtrih_m.fr_drv_ng.classic_interface.Classic;
import ru.shtrih_m.fr_drv_ng.classic_interface.ClassicImpl;


public class MainActivity extends AppCompatActivity {
    EditText m_editURL;
    Button m_btnGo;

    public static void checkResult(int ret) {
        if (ret != 0) {
            throw new RuntimeException("error, bad return code: " + ret);
        }
    }


    void exampleReceipt(){
        final Classic ci = new ClassicImpl();
        try {
            ci.Set_Password(30);
            ci.Set_ConnectionURI(m_editURL.getText().toString());
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
            ci.Set_StringForPrinting("Молоко");
            checkResult(ci.Sale());
            ci.Set_Summ1(10000);
            ci.Set_StringForPrinting("строчка");
            checkResult(ci.CloseCheck());
// Необходимо вручную отсоединяться от classic или форсить GC как в обработчике кнопки ниже.
// Иначе до вызова GC будет висеть соединение, а КЯ работает только с 1 соединением
//                    ci.Disconnect();
        } catch (Exception e) {
            ci.Disconnect();
            System.err.println("error:" + e.getLocalizedMessage());
        }
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        try {
            Libcore.os.setenv("FR_DRV_DEBUG_CONSOLE", "1", true);//выводим лог в logcat, а не в файл.
        } catch (Exception e) {
            e.printStackTrace();
        }

        m_editURL = findViewById(R.id.editText);
        m_editURL.setText("tcp://192.168.1.120:7778?timeout=1000&protocol=v1");
        m_btnGo = findViewById(R.id.button);
        m_btnGo.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                exampleReceipt();
                System.gc();
                System.runFinalization();
            }
        });
    }
}
