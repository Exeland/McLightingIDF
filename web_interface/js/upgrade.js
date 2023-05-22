function Mess(val){
    _("sate_load").innerHTML = val;
}

function upload() {
    let fileInput = _("newfile").files;

    if (fileInput.length == 0) {
        Mess("No file selected!");
        _("progressBar").value = 0; //wil clear progress bar after successful upload
    } else {
        fileInput.disabled = true;
        let file = fileInput[0];
        let xhttp = new XMLHttpRequest();
        xhttp.upload.addEventListener("progress", progressHandler, false);
        xhttp.upload.addEventListener("load", completeHandler, false);
        xhttp.upload.addEventListener("loadstart", startHandler, false);
        xhttp.upload.addEventListener("error", errorHandler, false);
        xhttp.upload.addEventListener("abort", abortHandler, false);
        xhttp.open("POST", "run?cmd=ota", true);
        xhttp.responseType = 'text';
        xhttp.send(file);
    }
}
function disableAll() {
    _("overlay").style.display = "block";
}

function enableAll() {
    _("overlay").style.display = "none";
}

function progressHandler(event) {
    _("loaded_n_total").innerHTML = "Uploaded " + event.loaded + " bytes of " + event.total;
    var percent = Math.round((event.loaded / event.total) * 100);
    _("progressBar").value = percent;
    Mess(percent + lang("proc_load")   );
}

function completeHandler(event) {
    let xhttp = event.target;
    let txt = xhttp.responseText === undefined ? "" : xhttp.responseText;
    enableAll();
    if (xhttp.status == 200) {
        Mess(txt);
        console.log(txt)
    } else {
        _("progressBar").value = 0;
        _("loaded_n_total").innerHTML = "";
        Mess(lang("error") + ": " + xhttp.sate_load + " \n" + txt);
    }
    setTimeout('redirect("/")', 5000)
}

function startHandler(event) {
    disableAll();
}

function errorHandler(event) {
    enableAll();
    Mess( "Upload Failed");
}

function abortHandler(event) {
    enableAll();
    Mess("Upload Aborted")
}

function file_change() {
    var file = _("newfile").files[0];
    if (file && file.name) {
        _('sel_file').innerHTML = file.name;
        _('btn_upld').disabled = false;
        _("progressBar").value = 0; //wil clear progress bar after successful upload
    } else {
        _('btn_upld').disabled = true;
    }
    Mess("");
    _("loaded_n_total").innerHTML = "";
}