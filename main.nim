import strutils,strformat,tables,os,times


type netItem = object
    name: string
    recv_bytes: int
    trans_bytes: int


proc readInfo(filename = "/proc/net/dev") : Table[string, netItem]= 
    var netItems : Table[string, netItem];
    for line in filename.lines:
        let arr=line.split(":")
        if len(arr) != 2 :
            continue;
        let parts = arr[1].splitWhitespace()
        if len(parts)!=16:
            continue;
        netItems[arr[0]]=netItem(name:arr[0],recv_bytes:parts[0].parseInt,trans_bytes:parts[8].parseInt)
    return netItems;

proc main() = 

    var netItemsLast : Table[string, netItem];
    var netItemsInit : Table[string, netItem];
    var init_clock:DateTime = now();
    var last_clock:DateTime = init_clock;
    var maxLen :int = 0;

    while true:
        let clock_now = now()
        let t = float((clock_now - last_clock).inMilliseconds)/1e3;
        let tt = float((clock_now - init_clock).inMilliseconds)/1e3;
        let netItemsCurrent = readInfo()
        if maxLen < 1:
            for ifname in netItemsCurrent.keys:
                if maxLen < ifname.len:
                    maxLen = ifname.len
        write(stdout,"\ec")
        for name,item in netItemsCurrent:
            let lastItem = netItemsLast.getOrDefault(name,netItem())
            let initItem = netItemsInit.getOrDefault(name,netItem())
            var total_recv  :float
            var total_trans  :float
            var recv:float
            var trans:float
            var recv_speed:float
            var trans_speed:float
            var recv_avg_speed:float
            var trans_avg_speed:float

            let first = lastItem.name.isEmptyOrWhitespace or initItem.name.isEmptyOrWhitespace
            if not first:

                total_recv = float(item.recv_bytes-initItem.recv_bytes)/1024
                total_trans = float(item.trans_bytes-initItem.trans_bytes)/1024

                recv = float(item.recv_bytes - lastItem.recv_bytes)/1024
                trans = float(item.trans_bytes - lastItem.trans_bytes)/1024

                recv_speed = recv/t
                trans_speed = trans/t

                recv_avg_speed = total_recv/tt
                trans_avg_speed = total_trans/tt

            let total_recv_mb = fmt"{total_recv/1024:.2f}MB"
            let recv_avg_speed_kb = fmt"{recv_avg_speed:.1f}KB/S"
            let recv_speed_kb = fmt"{recv_speed:.1f}KB/S"
            let total_trans_mb = fmt"{total_trans/1024:.2f}MB"
            let trans_avg_speed_kb = fmt"{trans_avg_speed:.1f}KB/S"
            let trans_speed_kb = fmt"{trans_speed:.1f}KB/S"

            let str_recv = fmt"{total_recv_mb:<11} {recv_avg_speed_kb:<11} {recv_speed_kb:<11}"
            let str_trans = fmt"{total_trans_mb:<11} {trans_avg_speed_kb:<11} {trans_speed_kb:<11}"
            let name_pad = name.alignLeft(maxLen)

            echo &"\e[1;34m{name_pad}\e[00m  接收: \e[1;32m{str_recv}\e[00m 发送: \e[1;31m{str_trans}\e[00m"

            continue;
        last_clock = clock_now
        netItemsLast = netItemsCurrent
        if netItemsInit.len < 1:
            netItemsInit = netItemsCurrent
            sleep(200)
        else:
            sleep(1000)
    

try:
    proc ctrlc() {.noconv.} =
        quit(0)
    setControlCHook(ctrlc)
    main()
except:
    echo getCurrentExceptionMsg()
