function reload(word) {
$.ajax({
    url: '/api/kv/' + word,
    success: function(result) {
        var el = $("#result").get(0);
        el.innerHTML = result;
    },
    error: function() {
        var el = $("#result").get(0);
        el.innerHTML = "";
    }
});
}

var el = $("#key").get(0);
el.addEventListener("input", function() {
    var prefix = $('#key').val();
    if (prefix.length>0) {
        reload(prefix);
    }
    else {
    	var el = $("#result").get(0);
        el.innerHTML = "";
    }
});
