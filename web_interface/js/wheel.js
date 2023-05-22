// ******************************************************************
// Colorwheel
// ******************************************************************
// this is supposed to work on mobiles (touch) as well as on a desktop (click)
// since we couldn't find a decent one .. this try of writing one by myself
// + google. swiping would be really nice - I will possibly implement it with
// jquery later - or never.

var canvas;
var context;

//comp to Hex
function componentToHex(c) {
    return ("0" + (Number(c).toString(16))).slice(-2).toUpperCase();
}

//rgb/rgba to Hex
function rgbToHex(rgb) {
    return componentToHex(rgb[0]) + componentToHex(rgb[1]) + componentToHex(rgb[2]);
}

function updateColorForm(hexColor){
    _("sel_color").style.backgroundColor = hexColor;
    _("sel_color").innerHTML = " (" + hexColor + ")";
}


//display the touch/click position and color info
function updateStatus(color) {
    var hexColor = rgbToHex(color);
    console.log("wsSetAll() Set all colors to:", hexColor);
    devColor = color;
    sendcolor();
    // hexColor = "#" + hexColor;

    updateColorForm("#" + hexColor)
}

//handle the touch event
function doTouch(event) {
    //to not also fire on click
    event.preventDefault();
    var el = event.target;

    //touch position
    var pos = {
        x: Math.round(event.targetTouches[0].pageX - el.offsetLeft),
        y: Math.round(event.targetTouches[0].pageY - el.offsetTop)
    };
    //color
    var color = context.getImageData(pos.x, pos.y, 1, 1).data;

    updateStatus(color);
}

//get Mouse x/y canvas position
function getMousePos(canvas, evt) {
    let rect = canvas.getBoundingClientRect();
    return {
        x: evt.clientX - rect.left,
        y: evt.clientY - rect.top
    };
}

function doClick(event) {
    //click position   
    let pos = getMousePos(canvas, event);
    //color
    let color = context.getImageData(pos.x, pos.y, 1, 1).data;

    console.log("click", pos.x, pos.y, color);
    updateStatus(color);

    //now do sth with the color rgbToHex(color);
    //don't do stuff when #000000 (outside circle and lines
}

function draw_wheel() {
    let centerX;
    let centerY;
    let innerRadius;
    let outerRadius;

    canvas = _("myCanvas");
    // FIX: Cancel touch end event and handle click via touchstart
    // canvas.addEventListener("touchend", function(e) { e.preventDefault(); }, false);
    canvas.addEventListener("touchmove", doTouch, false);
    canvas.addEventListener("click", doClick, false);
    //canvas.addEventListener("mousemove", doClick, false);

    context = canvas.getContext('2d');
    console.log(context);
    centerX = canvas.width / 2;
    centerY = canvas.height / 2;
    innerRadius = canvas.width / 5.5;
    outerRadius = (canvas.width - 10) / 2
    //outer border
    context.beginPath();
    //outer circle
    context.arc(centerX, centerY, outerRadius, 0, 2 * Math.PI, false);
    //draw the outer border: (gets drawn around the circle!)
    context.lineWidth = 1;
    context.strokeStyle = '#000000';
    context.stroke();
    context.closePath();

    //fill with beautiful colors 
    //taken from here: http://stackoverflow.com/questions/18265804/building-a-color-wheel-in-html5
    for (var angle = 0; angle <= 360; angle += 1) {
        var startAngle = (angle - 2) * Math.PI / 180;
        var endAngle = angle * Math.PI / 180;
        context.beginPath();
        context.moveTo(centerX, centerY);
        context.arc(centerX, centerY, outerRadius, startAngle, endAngle, false);
        context.closePath();
        context.fillStyle = 'hsl(' + angle + ', 100%, 50%)';
        context.fill();
        context.closePath();
    }
    
    //inner border
	context.beginPath();
	//context.arc(centerX, centerY, radius, startAngle, endAngle, counterClockwise);
	context.arc(centerX, centerY, innerRadius, 0, 2 * Math.PI, false);
	//fill the center
	let my_gradient=context.createLinearGradient(0,0,170,0);
	my_gradient.addColorStop(0,"black");
	my_gradient.addColorStop(1,"white");
	
	context.fillStyle = my_gradient;
	context.fillStyle = "white";
	context.fill();

	//draw the inner line
	context.lineWidth = 1;
	context.strokeStyle = '#000000';
	context.stroke();
	context.closePath();
}

const toRGBArray = rgbStr => rgbStr.match(/\d+/g).map(Number);
function SetBtnCol(BtnCol){
    updateStatus(toRGBArray(BtnCol));
}

const def_col = ["FF0000","FF8000","FFFF00","80FF00","00FF00","00FF80","00FFFF","0080FF","0000FF","8000FF","FF00FF","FF0080","FFFFFF"];

function addColorPalette() {
	let html = "<div>";
	for (let i = 0; i < def_col.length; i++) {
		let el = def_col[i];
		html += "<button type=\"button\" style=\"background: #" + el + "; margin: .5em\";\" onclick=\"SetBtnCol(style.backgroundColor)\">&nbsp;</button>";
	}
	html += "</div>";
	_("colorPalette").innerHTML = html;
}

function load() {
    draw_wheel();
    addColorPalette();
    refreshValues(function(){
        console.log(rgbToHex(devColor));
        updateColorForm("#" + rgbToHex(devColor));
    });
}