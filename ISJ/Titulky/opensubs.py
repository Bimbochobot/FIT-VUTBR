import xmlrpc.client
import xml.dom.minidom as xmlmini
import urllib.request
import subprocess
import base64
import gzip

class OpenSubDescriptor :
    def __init__(self, movie_name = None, imdbid = None, movie_hash = None, movie_byte_size = None, language = "cze") :
        self.data = {}
        if movie_name is not None and movie_name != "" :
            self.data['query'] = movie_name
        if imdbid is not None and imdbid != "":
            self.data['imdbid'] = imdbid
        if (movie_hash is not None and movie_byte_size is not None and movie_hash != "" and movie_byte_size > 0) :
            self.data['moviehash'] = movie_hash
            self.data['moviebytesize'] = movie_byte_size
        if self.data :
            self.data['sublanguageid'] = language
        else :
            raise ValueError("Invalid parameters")

class OpenSubtitles :
    _server_url = "http://api.opensubtitles.org/xml-rpc"
    _client = xmlrpc.client.ServerProxy(_server_url)

    def __init__(self, useragent) :
        self._token = self._client.LogIn("", "", "cze", useragent)['token']

    def __enter__(self) :
        return self

    def log_out(self) :
        self._client.LogOut(self._token)

    def search_subtitles(self, descriptors = None, url = None) :
        if descriptors is not None :
            links = self._client.SearchSubtitles(self._token, descriptors)
            return links['data']
        elif url is not None :
            xml = urllib.request.urlopen(url+"/xml")
            dom = xmlmini.parseString(xml.read())
            sub_info = dom.getElementsByTagName('Subtitle')
            general = {'SubtitlesLink' : url}
            links = []
            for x in sub_info  :
                if x.parentNode.nodeName == "SubBrowse" :
                    movie_name_node = x.getElementsByTagName('MovieName')[0]
                    general['MovieName'] = movie_name_node.firstChild.nodeValue
                    general['IDMovieImdb'] = movie_name_node.getAttribute('MovieIMDBID')
                    for y in x.childNodes :
                        if y.nodeName == "SubSumCD" :
                            general['SubSumCD'] = y.firstChild.nodeValue
                        elif y.nodeName == "SubCharEncoding" :
                            general['Encoding'] = y.firstChild.nodeValue
                        elif y.nodeName == "SubFormat" :
                            general['SubFormat'] = y.firstChild.nodeValue
                        elif y.nodeName == "IDSubtitle" :
                            general['IDSubtitle'] = y.firstChild.nodeValue
                        elif y.nodeName == "SubtitleFile" :
                            subs = y.getElementsByTagName("File")
                            for z in subs :
                                link = general
                                link['IDSubtitleFile'] = z.getAttribute("ID")
                                link['MovieByteSize'] = z.getElementsByTagName("MovieByteSize")[0].firstChild.nodeValue
                                links.append(link.copy())
            return links
        
    def get_info_from_xml(self, links) :
        for link in links :
            xml = urllib.request.urlopen(link['SubtitlesLink']+"/xml")
            dom = xmlmini.parseString(xml.read())
            sub_info = dom.getElementsByTagName('Subtitle')
            for x in sub_info  :
                if x.parentNode.nodeName == "SubBrowse" :
                    for y in x.childNodes :
                        if y.nodeName == "SubSumCD" :
                            link['SubSumCD'] = y.firstChild.nodeValue
                        elif y.nodeName == "SubCharEncoding" :
                            link['Encoding'] = y.firstChild.nodeValue
                        elif y.nodeName == "SubFormat" :
                            link['SubFormat'] = y.firstChild.nodeValue
                        elif y.nodeName == "SubtitleFile" :
                            m = [z for z in x.getElementsByTagName('File') if z.getAttribute("ID") == link['IDSubtitleFile']]
                            if m :
                                byte_size = m[0].getElementsByTagName("MovieByteSize")
                                if byte_size :
                                    link['MovieByteSize'] = byte_size[0].firstChild.nodeValue

    def download_subtitles(self, links) :
        ziped = self._client.DownloadSubtitles(self._token, [x['IDSubtitleFile'] for x in links])
        for i,x in enumerate(ziped['data']) :
            x['encoding'] = links[i].get('Encoding','cp1250')
        return [gzip.decompress(base64.b64decode(x['data'].encode('ascii'))).decode(x['encoding']) for x in ziped['data']]