var myAPIKey = '942fea5e2f107364c985ae435c55de53';

function fetchWeather(latitude, longitude) {
  var req = new XMLHttpRequest();
  req.open('GET', 'http://api.openweathermap.org/data/2.5/weather?' +
    'lat=' + latitude + '&lon=' + longitude + '&cnt=1&appid=' + myAPIKey, true);
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
        console.log(req.responseText);
        var response = JSON.parse(req.responseText);
        var temperature = Math.round((response.main.temp - 273.15) * (9.0 / 5.0) + 32.0);
        var temperatureCelcius = Math.round(response.main.temp - 273.15);
        console.log(temperature);
        Pebble.sendAppMessage({
          'weatherTemp': temperature + '\xB0F',
          'weatherTempCelcius': temperatureCelcius + '\xB0C'
        });
      } else {
        console.log('Error');
      }
    }
  };
  req.send(null);
}

function locationSuccess(pos) {
  var coordinates = pos.coords;
  fetchWeather(coordinates.latitude, coordinates.longitude);
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  Pebble.sendAppMessage({
    'weatherTemp': '--\xB0F'
  });
}

var locationOptions = {
  'timeout': 15000,
  'maximumAge': 60000,
  'enableHighAccuracy': true
};

Pebble.addEventListener('ready', function (e) {
  console.log('connect!' + e.ready);
  window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
  console.log(e.type);
});

Pebble.addEventListener('appmessage', function (e) {
  window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError, locationOptions);
  console.log(e.type);
  console.log('message!');
});

Pebble.addEventListener('showConfiguration', function() {
  Pebble.openURL('https://jvarness.github.io/sessions/');  
});

Pebble.addEventListener('webviewclosed', function(e) {
  if(e.response !== undefined && e.response !== null) {
    var configData = JSON.parse(decodeURIComponent(e.response));
  
    console.log('Recieved from app: ' + JSON.stringify(configData));
    
    if(configData.accentColor1){
      Pebble.sendAppMessage({
        "accentColor1": parseInt(configData.accentColor1, 16),
        "accentColor2": parseInt(configData.accentColor2, 16),
        "displayFahrenheit": configData.displayFahrenheit === true ? 1 : 0
      }); 
    }
  }
});