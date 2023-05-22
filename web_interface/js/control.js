var devColor = [0, 0, 0];
var devSpeed = 0;
var devBrghtns = 0;
var devmode = 0;

var cmnd_ready = true;
var las_cmnd = null;


function mess_cb(){
    if(las_cmnd != null){
        getFile(las_cmnd, mess_cb);
        cmnd_ready = false;
        las_cmnd = null;
    }
    else{
        cmnd_ready = true;
    }
}

function cmnd_send(mess){
    if(cmnd_ready){
        getFile(mess, mess_cb);
        cmnd_ready = false;       
    }
    else{
        las_cmnd = mess;
    }
}

function sendcolor(){
    console.log("Send color: " + devColor[0] + " " + devColor[1] + " " + devColor[2]);
    cmnd_send("run?cmd=setcolor " +  devColor[0] + " " + devColor[1] + " " + devColor[2]);
}

function setColR(r){devColor[0] = r; sendcolor()}
function setColG(g){devColor[1] = g; sendcolor()}
function setColB(b){devColor[2] = b; sendcolor()}

function setSpeed(s){
    devSpeed = s;
    console.log("Set speed:", devSpeed);
    cmnd_send("run?cmd=setspeed " +  devSpeed);
}

function setBrightness(brght){
    devBrghtns = brght;
    console.log("Set brightness:", devBrghtns);
    cmnd_send("run?cmd=setbrghtns " +  devBrghtns);
}

function set_mode(num){
    cmnd_send("run?cmd=setmode " + num);
}

function refreshValues(callback){
    getFile("state.json", function (res) {
        devColor = JSON.parse(res)["color"];
        devSpeed = JSON.parse(res)["speed"];
        devBrghtns = JSON.parse(res)["brghtns"];
        devmode = JSON.parse(res)["mode"];
        if (callback != undefined){
            callback();
        }
	});
}