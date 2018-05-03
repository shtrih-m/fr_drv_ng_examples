package ru.shtrih_m.classic_fr_drv_ng_android_example;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;

import libcore.io.Libcore;
import ru.shtrih_m.fr_drv_ng.android_util.UsbCdcAcmHelper;
import ru.shtrih_m.fr_drv_ng.classic_interface.Classic;
import ru.shtrih_m.fr_drv_ng.classic_interface.ClassicImpl;


public class MainActivity extends AppCompatActivity {
    EditText m_editURL;
    EditText m_editError;
    Button m_btnGo;
    Spinner m_uriSpinner;
    UsbCdcAcmHelper m_usbHelper;

    public static void checkResult(int ret) {
        if (ret != 0) {
            throw new RuntimeException("error, bad return code: " + ret);
        }
    }

    void exampleReceipt() {
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
            checkResult(ci.Disconnect());
// Необходимо вручную отсоединяться от classic или форсить GC как в обработчике кнопки ниже.
// Иначе до вызова GC будет висеть соединение, а КЯ работает только с 1 соединением
//                    ci.Disconnect();
        } catch (Exception e) {
            m_editError.setText(String.format("%d: %s", ci.Get_ResultCode(), ci.Get_ResultCodeDescription()));
            ci.Disconnect();
            System.err.println("error:" + e.getLocalizedMessage());

        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        m_usbHelper.unregister();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        m_usbHelper = new UsbCdcAcmHelper(this);
        m_usbHelper.register();
        try {
            Libcore.os.setenv("FR_DRV_DEBUG_CONSOLE", "1", true);//выводим лог в logcat, а не в файл.
        } catch (Exception e) {
            e.printStackTrace();
        }

        m_editURL = findViewById(R.id.uriEditText);
        m_editError = findViewById(R.id.error_text);
        m_btnGo = findViewById(R.id.button);
        m_btnGo.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                exampleReceipt();
                System.gc();
                System.runFinalization();
            }
        });
        m_uriSpinner = findViewById(R.id.spinner);

        final ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,
                R.array.uri_variants, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        m_uriSpinner.setAdapter(adapter);
        m_uriSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> adapterView, View view, int i, long l) {
                m_editURL.setText(adapter.getItem(i));
            }

            @Override
            public void onNothingSelected(AdapterView<?> adapterView) {

            }
        });
    }
}
