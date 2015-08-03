// This will be an array of dicts to send to Pebble
var messages = [];

// Function to send a message to the Pebble using AppMessage API
function sendMessage() {
  if (messages.length === 0) {
    return;
  }
  var message = messages.shift();
  Pebble.sendAppMessage(message, ack, nack);

  // If Pebble receives the message, send the next one
  function ack() {
    sendMessage();
  }

  // If Pebble doesn't receive the message, send it again
  function nack() {
    messages.unshift(message);
    sendMessage();
  }
}

function getFollowedStreams() {
  // Fetch request for user's followed streams
  var req = new XMLHttpRequest();
  req.open('GET', 'https://api.twitch.tv/kraken/streams/followed?oauth_token=' + localStorage.getItem('oauth token'), false);
  req.send(null);
  var message;
  var response = JSON.parse(req.responseText);
  // Number of streams/games sent to Pebble capped at 50
  var total = Math.min(response._total, 50);
  for (var i = 0; i < total; i++) {
    var streamer = response.streams[i].channel.display_name;
    var game = response.streams[i].game;
    // Construct array of dicts containing streams/games
    message = {
      1: streamer,
      2: game
    };
    messages.push(message);
  }
  sendMessage(message);
}

function getTopStreams() {
  // Fetch request for current top streams
  var req = new XMLHttpRequest();
  req.open('GET', 'https://api.twitch.tv/kraken/streams?limit=100', false);
  req.send(null);
  var message;
  var response = JSON.parse(req.responseText);
  // Number of streams/games sent to Pebble capped at 50
  var total = Math.min(response._total, 50);
  for (var i = 0; i < total; i++) {
    var streamer = response.streams[i].channel.display_name;
    var game = response.streams[i].game;
    // Construct array of dicts containing streams/games
    message = {
      1: streamer,
      2: game
    };
    messages.push(message);
  }
  sendMessage(message);
}

function getFeaturedStreams() {
  // Fetch request for current featured streams
  var req = new XMLHttpRequest();
  req.open('GET', 'https://api.twitch.tv/kraken/streams/featured?limit=100', false);
  req.send(null);
  var message;
  var response = JSON.parse(req.responseText);
  // Number of streams/games sent to Pebble capped at 50
  var total = Math.min(50, response.featured.length);
  for (var i = 0; i < total; i++) {
    var streamer = response.featured[i].stream.channel.name;
    var game = response.featured[i].stream.channel.game;
    message = {
      1: streamer,
      2: game
    };
    messages.push(message);
  }
  sendMessage(message);
}

// Configuration window
Pebble.addEventListener("showConfiguration", function(e) {
  Pebble.openURL('https://api.twitch.tv/kraken/oauth2/authorize?response_type=token&client_id=kqxn6nov00how5uom46vlxb7p32xvf6&redirect_uri=pebblejs://close&scope=user_read');
});

// Called when the configuration window is closed
Pebble.addEventListener("webviewclosed", function(e) {});

// Called when Pebble first starts
Pebble.addEventListener("ready", function(e) {
  // Store a sample oauth token into local storage for now
  var oauth_token = '0z7aboxelw2npt53h9ax0vxcwutrox';
  localStorage.setItem('oauth token', oauth_token);
});

// Called when incoming message from the Pebble is received
Pebble.addEventListener("appmessage", function(e) {
  if (e.payload.QUERY_KEY === 0) {
    getFollowedStreams();
  } else if (e.payload.QUERY_KEY === 1) {
    getTopStreams();
  } else if (e.payload.QUERY_KEY === 2) {
    getFeaturedStreams();
  }
});
