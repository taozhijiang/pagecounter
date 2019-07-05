
// url   POST destination address
// id    account_id
// elem  element, update its innerTHML
function counter_report(url, id, elem) {

    var post_data = {
        "id"    : id,
        "host"  : window.location.host,
        "uri"   : window.location.pathname,
        "origin": window.location.origin,
    };
    var content = JSON.stringify(post_data);

    // XMLHttpRequest better compatiable with legacy-browser
    var xhr = new XMLHttpRequest();
    xhr.open("POST", url, true);
    xhr.setRequestHeader('Content-type','application/json; charset=utf-8');

    xhr.onload = function () {
        var counts = JSON.parse(xhr.responseText);
        if (xhr.readyState == 4 && xhr.status == "200") {
            page_counter.innerHTML = xhr.responseText;
        } else {
            console.error(counts);
        }
    }
    xhr.send(content);
}