var langJson = {};

const Menu_List_tab = ["modes", "wheel","settings","about"]

function _(name) {
	return document.getElementById(name);
}

function esc(str) {
	if (str) {
		return str.toString()
			.replace(/&/g, '&amp;')
			.replace(/</g, '&lt;')
			.replace(/>/g, '&gt;')
			.replace(/\"/g, '&quot;')
			.replace(/\'/g, '&#39;')
			.replace(/\//g, '&#x2F;');
	}
	return "";
}

function convertLineBreaks(str) {
	if (str) {
		str = str.toString();
		str = str.replace(/(?:\r\n|\r|\n)/g, '<br>');
		return str;
	}
	return "";
}

function showMessage(msg) {
	if (msg.startsWith("ERROR")) {
		_("status").style.backgroundColor = "#d33";
		_("status").innerHTML = lang("disconnected");

		console.error("disconnected (" + msg + ")");
	} else if (msg.startsWith("LOADING")) {
		_("status").style.backgroundColor = "#fc0";
		_("status").innerHTML = lang("loading");
	} else {
		_("status").style.backgroundColor = "#3c5";
		_("status").innerHTML = lang("connected");

		console.log("" + msg + "");
	}
}

function getFile(adr, callback, timeout, method, onTimeout, onError) {
	/* fallback stuff */
	if (adr === undefined) return;
	if (callback === undefined) callback = function () { };
	if (timeout === undefined) timeout = 8000;
	if (method === undefined) method = "GET";
	if (onTimeout === undefined) {
		onTimeout = function () {
			showMessage("ERROR: timeout loading file " + adr);
		};
	}
	if (onError === undefined) {
		onError = function () {
			showMessage("ERROR: loading file: " + adr);
		};
	}

	/* create request */
	var request = new XMLHttpRequest();

	/* set parameter for request */
	request.open(method, encodeURI(adr), true);
	request.timeout = timeout;
	request.ontimeout = onTimeout;
	request.onerror = onError;
	request.overrideMimeType("application/json");

	request.onreadystatechange = function () {
		if (this.readyState == 4) {
			if (this.status == 200) {
				// setTimeout('showMessage("connected")', 100);
				showMessage("connected");
				callback(this.responseText);
			}
		}
	};

	/* send request */
	request.send();
	console.log(adr);
}

function lang(key) {
	return convertLineBreaks(esc(langJson[key] ? langJson[key] : key));
}

function parseLang(fileStr) {
	langJson = JSON.parse(fileStr);
    var elements = document.querySelectorAll("[data-translate]");
    for (i = 0; i < elements.length; i++) {
        let element = elements[i];
        let translation = lang(element.getAttribute("data-translate"))
        if (translation) {
            element.innerHTML = translation;
        }
    }
	document.querySelector('html').setAttribute("lang", langJson["lang"]);
	if (typeof (load) === 'function') load();
}

function drawmenu() {
	let html = "<nav><ul class=\"menu\">";
	for (let i = 0; i < Menu_List_tab.length; i++) {
		let el = Menu_List_tab[i];
		html += "<li><a href=\"" + el + ".html\" data-translate=\"" + el + "\">" + el + "</a></li>";
	}
	html += "</nav>";
	_("menuList").innerHTML = html;
}

function loadLang() {
	if (_("menuList")) drawmenu();
	getFile("lang/default.lang",
		parseLang,
		2000,
		"GET",
		function () {
			getFile("lang/en.lang", parseLang);
		}, function () {
			getFile("lang/en.lang", parseLang);
		}
	);
}
function redirect(url) {
	window.location.href = url;
}

window.addEventListener('load', function () {
	_("status").style.backgroundColor = "#3c5";
	_("status").innerHTML = "connected";
})