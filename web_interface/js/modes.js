var modes = [];


function draw_modes() {
    let html = "";
    for (let i = 0; i < modes.length; i++) {
        let name =  modes[i];
        console.log("mode " + i + ":", name);
        html += "<button onclick=\"set_mode("+ i + ")\" class=\"col-12 centered\">" + name +"</button>" 
    }
    _("modes").innerHTML = html;    
}

function send_off(){
    console.log("send off");
}

function send_tv(){
    console.log("send tv");
}

function load() {
	getFile("modes.json", function (res) {
        modes = JSON.parse(res)["names"];
		showMessage("connected");
		draw_modes();
	});

    refreshValues(function(){
        _("rng_red").value = devColor[0];
        _("rng_green").value = devColor[1];
        _("rng_blue").value = devColor[2];
        _("rng_delay").value = devSpeed;
        _("rng_brightness").value = devBrghtns;
    });
}