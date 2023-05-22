var settingsJson = {};

function load() {
	getFile("settings.json", function (res) {
		settingsJson = JSON.parse(res);
		showMessage("connected");
		draw();
	});
}

function draw() {
	let html = "";

	for (let key in settingsJson) {
		key = esc(key);
		if (settingsJson.hasOwnProperty(key)) {
			html += "<h3>" + lang("setting_name_" + key) + "</h3>";
			for (let key1 in settingsJson[key]) {
				key1 = esc(key1);
				if (settingsJson[key].hasOwnProperty(key1)) {
					let translate_name = lang("setting_name_" + key + "_" + key1);
					if(!translate_name){
						translate_name = key1;
					}
					html += "<div class='row'>"
						+ "<div class='col-6'>"
						+ "<label class='settingName " + (typeof settingsJson[key][key1] == "boolean" ? "labelFix" : "") + "' for='" + key1 + "'>" + translate_name + ":</label>"
						+ "</div>"
						+ "<div class='col-6'>";

					if (typeof settingsJson[key][key1] == "boolean") {
						html += "<label class='checkBoxContainer'><input type='checkbox' name='" + key1 + "' " + (settingsJson[key][key1] ? "checked" : "") + " onchange='save(\"" + key + "\",\"" + key1 + "\",!settingsJson[\"" + key + "\"][\"" + key1 + "\"])'><span class='checkmark'></span></label>";
					} else if (typeof settingsJson[key][key1] == "number") {
						html += "<input type='number' name='" + key1 + "' value=" + settingsJson[key][key1] + " onchange='save(\"" + key + "\",\"" + key1 + "\",parseInt(this.value))'>";
					} else if (typeof settingsJson[key][key1] == "string") {
						html += "<input type='text' name='" + key1 + "' value='" + settingsJson[key][key1].toString() + "' " + (key1 == "version" ? "readonly" : "") + " onchange='save(\"" + key + "\",\"" + key1 + "\",this.value)'>";
					}

					html += "</div>"
						+ "</div>"
						+ "<div class='row'>"
						+ "<div class='col-12'>"
						+ "<p>" + lang("setting_desc_" + key + "_" + key1) + "</p>"
						+ "</div>"
						+ "</div>";
				}
			}
			html += "<hr>";
		}
	}
	_("settingsList").innerHTML = html;
}

function save(key, key1, value) {
	if (key && key1) {
		settingsJson[key][key1] = value;
		getFile("run?cmd=set " + key + "." + key1 + " \"" + value + "\"");
	} else {
		getFile("run?cmd=save settings", function (res) {
			load();
		});
	}
}

function reset() {
	getFile("run?cmd=reset");
	setTimeout('redirect("index.html")', 3000)
}