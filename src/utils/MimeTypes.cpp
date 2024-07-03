# include "MimeTypes.hpp"

MimeTypes::MimeTypes() {
  _mimes[".html"] = "text/html";
  _mimes[".htm"] = "text/html";
  _mimes[".shtml"] = "text/html";
  _mimes[".css"] = "text/css";
  _mimes[".xml"] = "text/xml";
  _mimes[".gif"] = "image/gif";
  _mimes[".jpeg"] = "image/jpeg";
  _mimes[".jpg"] = "image/jpeg";
  _mimes[".js"] = "application/javascript";
  _mimes[".atom"] = "application/atom+xml";
  _mimes[".rss"] = "application/rss+xml";

  _mimes[".mml"] = "text/mathml";
  _mimes[".txt"] = "text/plain";
  _mimes[".jad"] = "text/vnd.sun.j2me.app-descriptor";
  _mimes[".wml"] = "text/vnd.wap.wml";
  _mimes[".htc"] = "text/x-component";

  _mimes[".png"] = "image/png";
  _mimes[".svg"] = "image/svg+xml";
  _mimes[".svgz"] = "image/svg+xml";
  _mimes[".tif"] = "image/tiff";
  _mimes[".tiff"] = "image/tiff";
  _mimes[".wbmp"] = "image/vnd.wap.wbmp";
  _mimes[".webp"] = "image/webp";
  _mimes[".ico"] = "image/x-icon";
  _mimes[".jng"] = "image/x-jng";
  _mimes[".bmp"] = "image/x-ms-bmp";

  _mimes[".woff"] = "font/woff";
  _mimes[".woff2"] = "font/woff2";

  _mimes[".jar"] = "application/java-archive";
  _mimes[".war"] = "application/java-archive";
  _mimes[".ear"] = "application/java-archive";
  _mimes[".json"] = "application/json";
  _mimes[".hqx"] = "application/mac-binhex40";
  _mimes[".doc"] = "application/msword";
  _mimes[".pdf"] = "application/pdf";
  _mimes[".ps"] = "application/postscript";
  _mimes[".eps"] = "application/postscript";
  _mimes[".ai"] = "application/postscript";
  _mimes[".rtf"] = "application/rtf";
  _mimes[".m3u8"] = "application/vnd.apple.mpegurl";
  _mimes[".kml"] = "application/vnd.google-earth.kml+xml";
  _mimes[".kmz"] = "application/vnd.google-earth.kmz";
  _mimes[".xls"] = "application/vnd.ms-excel";
  _mimes[".eot"] = "application/vnd.ms-fontobject";
  _mimes[".ppt"] = "application/vnd.ms-powerpoint";
  _mimes[".odg"] = "application/vnd.oasis.opendocument.graphics";
  _mimes[".odp"] = "application/vnd.oasis.opendocument.presentation";
  _mimes[".ods"] = "application/vnd.oasis.opendocument.spreadsheet";
  _mimes[".odt"] = "application/vnd.oasis.opendocument.text";

  _mimes[".pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
  _mimes[".xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
  _mimes[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";

  _mimes[".wmlc"] = "application/vnd.wap.wmlc";
  _mimes[".7z"] = "application/x-7z-compressed";
  _mimes[".cco"] = "application/x-cocoa";
  _mimes[".jardiff"] = "application/x-java-archive-diff";
  _mimes[".jnlp"] = "application/x-java-jnlp-file";
  _mimes[".run"] = "application/x-makeself";
  _mimes[".pl"] = "application/x-perl";
  _mimes[".pm"] = "application/x-perl";
  _mimes[".prc"] = "application/x-pilot";
  _mimes[".pdb"] = "application/x-pilot";
  _mimes[".rar"] = "application/x-rar-compressed";
  _mimes[".rpm"] = "application/x-redhat-package-manager";
  _mimes[".sea"] = "application/x-sea";
  _mimes[".swf"] = "application/x-shockwave-flash";
  _mimes[".sit"] = "application/x-stuffit";
  _mimes[".tcl"] = "application/x-tcl";
  _mimes[".tk"] = "application/x-tcl";
  _mimes[".der"] = "application/x-x509-ca-cert";
  _mimes[".pem"] = "application/x-x509-ca-cert";
  _mimes[".crt"] = "application/x-x509-ca-cert";
  _mimes[".xpi"] = "application/x-xpinstall";
  _mimes[".xhtml"] = "application/xhtml+xml";
  _mimes[".xspf"] = "application/xspf+xml";
  _mimes[".zip"] = "application/zip";

  _mimes[".bin"] = "application/octet-stream";
  _mimes[".exe"] = "application/octet-stream";
  _mimes[".dll"] = "application/octet-stream";
  _mimes[".deb"] = "application/octet-stream";
  _mimes[".dmg"] = "application/octet-stream";
  _mimes[".iso"] = "application/octet-stream";
  _mimes[".img"] = "application/octet-stream";
  _mimes[".msi"] = "application/octet-stream";
  _mimes[".msp"] = "application/octet-stream";
  _mimes[".msm"] = "application/octet-stream";

  _mimes[".mid"] = "audio/midi";
  _mimes[".midi"] = "audio/midi";
  _mimes[".kar"] = "audio/midi";
  _mimes[".mp3"] = "audio/mpeg";
  _mimes[".ogg"] = "audio/ogg";
  _mimes[".m4a"] = "audio/x-m4a";
  _mimes[".ra"] = "audio/x-realaudio";

  _mimes[".3gpp"] = "video/3gpp";
  _mimes[".3gp"] = "video/3gpp";
  _mimes[".ts"] = "video/mp2t";
  _mimes[".mp4"] = "video/mp4";
  _mimes[".mpeg"] = "video/mpeg";
  _mimes[".mpg"] = "video/mpeg";
  _mimes[".mov"] = "video/quicktime";
  _mimes[".webm"] = "video/webm";
  _mimes[".flv"] = "video/x-flv";
  _mimes[".m4v"] = "video/x-m4v";
  _mimes[".mng"] = "video/x-mng";
  _mimes[".asx"] = "video/x-ms-asf";
  _mimes[".asf"] = "video/x-ms-asf";
  _mimes[".wmv"] = "video/x-ms-wmv";
  _mimes[".avi"] = "video/x-msvideo";
}

std::string MimeTypes::getType(std::string extension) {
  if (!extension.length())
    return "application/octet-stream";
  if (_mimes.count(extension))
    return _mimes[extension];
  return "application/octet-stream";
};

std::string MimeTypes::getExtension(std::string content_type) {
  
  for (std::map<std::string, std::string>::iterator it = _mimes.begin(); it != _mimes.end(); it++) {
    if (it->second == content_type) {
      return it->first;   
    }
  }
  return ".bin";

}