using System;
using ru.shtrih_m.fr_drv_ng.classic_interface;
namespace classic_csharp
{
    class Program
    {
        static void checkResult(int code)
        {
            if (code != 0)
            {
                throw new Exception("bad result code");
            }
        }
        static void Main(string[] args)
        {
            var ci = new classic_interface();
            Console.WriteLine(ci.RandomSequence);
            ci.ConnectionURI = "tcp://192.168.137.111:7778";
            Console.WriteLine(ci.ConnectionURI);
            checkResult(ci.Connect());
            checkResult(ci.GetECRStatus());
            Console.WriteLine(String.Format("{0}: {1}", ci.ECRMode, ci.ECRModeDescription));
        }
    }
}
