#include "channels.h"

void ChannelsResponder::reply(std::ostream& out, cxxtools::http::Request& request, cxxtools::http::Reply& reply)
{
  if ( request.method() != "GET") {
     reply.httpReturn(403, "To retrieve information use the GET method!");
     return;
  }

  std::string params = getRestParams((std::string)"/channels", request.url()); 
  ChannelList* channelList;

  if ( isFormat(params, ".json") ) {
    reply.addHeader("Content-Type", "application/json; charset=utf-8");
    channelList = (ChannelList*)new JsonChannelList(&out);
  } else if ( isFormat(params, ".html") ) {
    reply.addHeader("Content-Type", "text/html; charset=utf-8");
    channelList = (ChannelList*)new HtmlChannelList(&out);
  } else if ( isFormat(params, ".xml") ) {
    reply.addHeader("Content-Type", "text/xml; charset=utf-8");
    channelList = (ChannelList*)new XmlChannelList(&out);
  } else {
    reply.httpReturn(403, "Resources are not available for the selected format. (Use: .json, .html or .xml)");
    return;
  }

  channelList->init();

  for (cChannel *channel = Channels.First(); channel; channel = Channels.Next(channel))
  {
    if (!channel->GroupSep()) {
       channelList->addChannel(channel);
    }
  }

  channelList->finish();
  delete channelList;
}

void operator<<= (cxxtools::SerializationInfo& si, const SerChannel& c)
{
  si.addMember("name") <<= c.Name;
  si.addMember("number") <<= c.Number;
  si.addMember("transponder") <<= c.Transponder;
  si.addMember("stream") <<= c.Stream;
  si.addMember("is_atsc") <<= c.IsAtsc;
  si.addMember("is_cable") <<= c.IsCable;
  si.addMember("is_terr") <<= c.IsTerr;
  si.addMember("is_sat") <<= c.IsSat;
}

void operator<<= (cxxtools::SerializationInfo& si, const SerChannels& c)
{
  si.addMember("rows") <<= c.channel;
}

void HtmlChannelList::init()
{
  writeHtmlHeader(out);

  write(out, "<ul>");
}

void HtmlChannelList::addChannel(cChannel* channel)
{
  write(out, "<li>");
  write(out, (char*)channel->Name());
  write(out, "\n");
}

void HtmlChannelList::finish()
{
  write(out, "</ul>");
  write(out, "</body></html>");
}

void JsonChannelList::addChannel(cChannel* channel)
{
  std::string suffix = (std::string) ".ts";

  SerChannel serChannel;
  serChannel.Name = UTF8Decode(channel->Name());
  serChannel.Number = channel->Number();
  serChannel.Transponder = channel->Transponder();
  serChannel.Stream = UTF8Decode(((std::string)channel->GetChannelID().ToString() + (std::string)suffix).c_str());
  serChannel.IsAtsc = channel->IsAtsc();
  serChannel.IsCable = channel->IsCable();
  serChannel.IsSat = channel->IsSat();
  serChannel.IsTerr = channel->IsTerr();
  serChannels.push_back(serChannel);
}

void JsonChannelList::finish()
{
  cxxtools::JsonSerializer serializer(*out);
  serializer.serialize(serChannels, "channels");
  serializer.finish();
}

void XmlChannelList::init()
{
  write(out, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
  write(out, "<channels xmlns=\"http://www.domain.org/restfulapi/2011/channels-xml\">\n");
}

void XmlChannelList::addChannel(cChannel* channel)
{
  std::string suffix = (std::string) ".ts";

  write(out, " <channel>\n");
  write(out, (const char*)cString::sprintf("  <param name=\"name\">%s</param>\n", encodeToXml(channel->Name()).c_str()));
  write(out, (const char*)cString::sprintf("  <param name=\"number\">%i</param>\n", channel->Number()));
  write(out, (const char*)cString::sprintf("  <param name=\"transponder\">%i</param>\n", channel->Transponder()));
  write(out, (const char*)cString::sprintf("  <param name=\"stream\">%s</param>\n", encodeToXml( ((std::string)channel->GetChannelID().ToString() + (std::string)suffix).c_str()).c_str()));
  write(out, (const char*)cString::sprintf("  <param name=\"is_atsc\">%s</param>\n", channel->IsAtsc() ? "true" : "false"));
  write(out, (const char*)cString::sprintf("  <param name=\"is_cable\">%s</param>\n", channel->IsCable() ? "true" : "false"));
  write(out, (const char*)cString::sprintf("  <param name=\"is_sat\">%s</param>\n", channel->IsSat() ? "true" : "false"));
  write(out, (const char*)cString::sprintf("  <param name=\"is_terr\">%s</param>\n", channel->IsTerr() ? "true" : "false"));
  write(out, " </channel>\n");
}

void XmlChannelList::finish()
{
  write(out, "</channels>");
}
