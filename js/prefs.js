(function() {
	loadOptions();
	submitHandler();
})();

function loadOptions() {
  var $accent1 = $('#accentColor1');
  var $accent2 = $('#accentColor2');
  
  if (localStorage.accentColor1) {
    $accent1[0].value = localStorage.accentColor1;
    $accent2[0].value = localStorage.accentColor2;
  }
}

function submitHandler() {
	var $submitButton = $('#submit');
	
	$submitButton.on('click', handleSubmit);
}

function getAndStoreConfig() {
  var $accent1 = $('#accentColor1');
  var $accent2 = $('#accentColor2');
  
  var options = {
    accentColor1: $accent1.val(),
    accentColor2: $accent2.val()
  };
  
  localStorage.accentColor1 = options.accentColor1;
  localStorage.accentColor2 = options.accentColor2;
  
  return options;
}

function getQueryParam(variable, defaultValue) {
  var query = location.search.substring(1);
  var vars = query.split('&');
  for (var i = 0; i < vars.length; i++) {
    var pair = vars[i].split('=');
    if (pair[0] === variable) {
      return decodeURIComponent(pair[1]);
    }
  }
  return defaultValue || false;
}

function handleSubmit() {
  var return_to = getQueryParam('return_to', 'pebblejs://close#');
  document.location = return_to + encodeURIComponent(JSON.stringify(getAndStoreConfig()));
}