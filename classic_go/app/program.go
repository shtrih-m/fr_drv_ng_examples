package main

import (
	"classic_fr_drv_ng"
	"fmt"
)

// #cgo CFLAGS: -I.
// #cgo LDFLAGS: -L. -lclassic_fr_drv_ng_go
import "C"

func checkError(err int) error {
	if err != 0 {
		return fmt.Errorf("bad return code: %d", err)
	}
	return nil
}

func main() {
	ci := classic_fr_drv_ng.NewClassic_interface()
	ci.Set_ConnectionURI("tcp://192.168.137.111:7778?timeout=30048&plain_transfer=auto")
	checkError(ci.Connect())
	checkError(ci.GetECRStatus())
	fmt.Println("mode: ", ci.Get_ECRMode(), ci.Get_ECRModeDescription())

}
