var http = require('http'),
    fileSystem = require('fs'),
    path = require('path'),
    util = require('util');

http.createServer(function(request, response) {
    var filePath = 'resources/labels.mp3';
    var stat = fileSystem.statSync(filePath);

    response.writeHead(200, {
        'Content-Type': 'audio/mp3',
        'Content-Length': stat.size
    });

    var readStream = fileSystem.createReadStream(filePath);
    readStream.pipe(response);
})
.listen(2000);
