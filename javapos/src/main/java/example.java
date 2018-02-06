import jpos.FiscalPrinter;
import jpos.JposException;
import jpos.config.JposEntry;
import jpos.config.JposEntryRegistry;
import jpos.config.simple.SimpleEntry;
import jpos.loader.JposServiceLoader;

public class example {
    public static void main(String[] args) {
        System.out.println(System.getProperty("java.library.path"));
        //в java.library.path должна быть директория с dll драйвера.
        try {
            SimpleEntry entry = new SimpleEntry();
            entry.addProperty(JposEntry.SI_FACTORY_CLASS_PROP_NAME, "ru.shtrih_m.fr_drv_ng.jpos.FiscalPrinterServiceNgFactory");
            entry.addProperty(JposEntry.SERVICE_CLASS_PROP_NAME, "ru.shtrih_m.fr_drv_ng.jpos.FiscalPrinterServiceNg");
            entry.addProperty(JposEntry.LOGICAL_NAME_PROP_NAME, "shtrihfiscalprinter");
            entry.addProperty("io_url", "tcp://192.168.137.111:7778?timeout=1000&protocol=v1");
            entry.addProperty("io_trace", true);
            JposEntryRegistry registry = JposServiceLoader.getManager().getEntryRegistry();
            registry.addJposEntry(entry);
            FiscalPrinter printer = new FiscalPrinter();
            printer.open("shtrihfiscalprinter");
            printer.claim(0);
            printer.setDeviceEnabled(true);
            printer.beginFiscalReceipt(false);
            printer.printRecItem("Молоко", 10000,1000,1,10000,"");
            printer.printRecTotal(10000,10000,"1");
            printer.endFiscalReceipt(false);
            System.exit(0);
        } catch (Exception e) {
            System.err.println("error:" + e.getLocalizedMessage());
        }
        System.exit(1);
    }
}
