
// url   POST destination address
// id    account_id
// elem  element, update its innerTHML
function counter_report(url, id, elem) {

    var post_data = {
        "id"    : id,
        "host"  : window.location.host,
        "uri"   : window.location.pathname,
        "proto" : window.location.protocol,
        "origin": window.location.origin,
        "browser"  : window.navigator.userAgent,
        "platform" : window.navigator.platform,
        "language" : window.navigator.language,
    };
    var content = JSON.stringify(post_data);

    // XMLHttpRequest better compatiable with legacy-browser
    var xhr = new XMLHttpRequest();
    xhr.open("POST", url, true);
    xhr.setRequestHeader('Content-type','application/json; charset=utf-8');

    xhr.onload = function () {
        if (xhr.readyState == 4 && xhr.status == "200") {
            page_counter.innerHTML = xhr.responseText;
        } else {
            console.error(xhr.responseText);
        }
    }
    xhr.send(content);
}