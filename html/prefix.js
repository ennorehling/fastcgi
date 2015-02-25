function removeAll(el) {
    while (el.firstChild) {
        el.removeChild(el.firstChild);
    }
}

function reload(word) {
$.ajax({
    url: '/cgi/' + word,
    success: function(result) {
        console.log(typeof(result));
        words = result;
        var el = $("#wordlist").get(0);
        removeAll(el);
        words.forEach(function(word, index) {
            var li = document.createElement('li');
            li.innerHTML = word;
            el.appendChild(li);
        })},
    dataType: "json",
    contentType: "application/json",
    accepts: {
        "json": "application/json"
    }
});
}

var el = $("#input").get(0);
console.log(el);
el.addEventListener("input", function() {
    var prefix = $('#input').val();
    if (prefix.length>0) {
        reload(prefix);
    }
    else {
    	var el = $("#wordlist").get(0);
        removeAll(el);
    }
});
