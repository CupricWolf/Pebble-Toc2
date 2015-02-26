//    file: pebble-js-app.js
//    auth: Matthew Clark, SetPebble

// change this token for your project
var setPebbleToken = 'ZECS';
var uuid = "0a88492a-12b6-4c72-8bd5-6e01993f0a58";
var version = "2.3";

function versionCheck(uuid, version) {
    console.log("Checking version.");
    var url = 'http://pblweb.com/api/v1/version/' + uuid + '.json?current=' + version;
    var req = new XMLHttpRequest();
    req.open('GET', url, true);
    req.onload = function (e) {
        if (req.readyState === 4 && req.status === 200) {
            if (req.status === 200) {
                var response = JSON.parse(req.responseText);
                var version = response.version;
                console.log("Appstore Version:" + version);
                var newer = response.newer;
                if (newer) {
                    Pebble.showSimpleNotificationOnPebble('New version!', 'A new version (' + version + ') of my amazing app is available.');
                    console.log("Newer version exists!");
                }
            }
        }
    };
    req.send(null);
}

Pebble.addEventListener('ready', function (e) {
    versionCheck(uuid, version);
    var settings = localStorage.getItem(setPebbleToken);
        if (typeof(settings) == 'string') {
            console.log("Sending settings from localStorage.");
            try {
                Pebble.sendAppMessage(JSON.parse(settings));
            } catch (e) {
            }
        }
        var request = new XMLHttpRequest();
        request.open('GET', 'http://x.SetPebble.com/api/' + setPebbleToken + '/' + Pebble.getAccountToken(), true);
        request.onload = function (e) {
            if (request.readyState === 4)
                if (request.status === 200) {
                    console.log("Sending settings from setPebbleAPI.");
                    try {
                        Pebble.sendAppMessage(JSON.parse(request.responseText));
                    } catch (e) {
                    }
        }
        };
        request.send(null);
        console.log("Settings sent.");
});
Pebble.addEventListener('appmessage', function (e) {
    key = e.payload.action;
    if (typeof(key) != 'undefined') {
        var settings = localStorage.getItem(setPebbleToken);
        if (typeof(settings) == 'string') {
            console.log("Sending settings from localStorage.");
            try {
                Pebble.sendAppMessage(JSON.parse(settings));
            } catch (e) {
            }
        }
        var request = new XMLHttpRequest();
        request.open('GET', 'http://x.SetPebble.com/api/' + setPebbleToken + '/' + Pebble.getAccountToken(), true);
        request.onload = function (e) {
            if (request.readyState === 4)
                if (request.status === 200) {
                    console.log("Sending settings from setPebbleAPI.");
                    try {
                        Pebble.sendAppMessage(JSON.parse(request.responseText));
                    } catch (e) {
                    }
        }
        };
        request.send(null);
        console.log("Settings sent.");
    }
});
Pebble.addEventListener('showConfiguration', function (e) {
    Pebble.openURL('http://x.SetPebble.com/' + setPebbleToken + '/' + Pebble.getAccountToken());
});
Pebble.addEventListener('webviewclosed', function (e) {
    if ((typeof(e.response) == 'string') && (e.response.length > 0)) {
        try {
            Pebble.sendAppMessage(JSON.parse(e.response));
            localStorage.setItem(setPebbleToken, e.response);
        } catch(e) {
        }
    }
});