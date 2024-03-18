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
import ru.shtrih_m.classic_fr_drv_ng_example.ExampleStuff;
import ru.shtrih_m.fr_drv_ng.android_util.StaticContext;
import ru.shtrih_m.fr_drv_ng.android_util.UsbCdcAcmHelper;
import ru.shtrih_m.fr_drv_ng.classic_interface.classic_interface;


public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MainActivity";
    EditText m_editURL;
    EditText m_editError;
    Button m_btnGo;
    Spinner m_uriSpinner;
    UsbCdcAcmHelper m_usbHelper;


    void exampleReceipt() {
// т.к. интерфейс единый, а java - это только обёртка вокруг c++ класса
// стоит так же смотреть пример c++. В основном модифицироваться будет он.
// https://git.shtrih-m.ru/fr_drv_ng/examples/blob/master/classic_interface/main.cpp
        final classic_interface ci = new classic_interface();
        try {
            ci.Set_Password(1);
            ci.Set_SysAdminPassword(30);
            ci.Set_ConnectionURI(m_editURL.getText().toString());
            ExampleStuff.checkResult(ci.Connect()); //соединяемся
            ci.Set_WaitForPrintingDelay(20); //задержка ожидания окончания печати
            ExampleStuff.prepareReceipt(ci);
            ExampleStuff.fsOperationReceipt(ci);
            m_editError.setText(String.format("%d: %s", ci.Get_ResultCode(), ci.Get_ResultCodeDescription()));
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
        StaticContext.setContextFromActivity(this); //чтобы автоматически запрашивать разрешения при попытке установить связь по bluetooth
        m_usbHelper = new UsbCdcAcmHelper(this);
        m_usbHelper.register();

        try {
            Libcore.os.setenv("FR_DRV_DEBUG_CONSOLE", "1", true);//выводим лог в logcat, а не в файл.
//            String LOG_FILE = Environment.getExternalStorageDirectory().getPath() + File.separator+"fr_drv_ng.log";
//            Libcore.os.setenv("FR_DRV_LOG_PATH", LOG_FILE, true);
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
