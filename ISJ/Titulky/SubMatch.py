import subrip
import opensubs
import re
import sys
import argparse

class SubMatch :
    def __init__ (self, subA, subB) :
        self._a_speeches = subA.make_speeches()
        self._b_speeches = subB.make_speeches()

    def _make_iter(self) :
        if self._a_next :
            text = ""
            start = end = None
            self._a_x = None
            while re.match('.*(?:\.{1,3}|\?|!)$', text) is None and self._a_it is not None:
                try :
                    item = next(self._a_it)
                except StopIteration :
                    self._a_it = None
                    return False
                if start is None :
                    start = item[0]
                if text :
                    text += " "
                text += item[2].strip()
                end = item[1]
                break # iba 1 iteracia
            self._a_x = (start, end, text)

        if self._b_next :
            text = ""
            start = end = None
            self._b_x = None
            while re.match('.*(?:\.{1,3}|\?|!)$', text) is None and self._b_it is not None:
                try :
                    item = next(self._b_it)
                except StopIteration :
                    self._b_it = None
                    return False
                if start is None :
                    start = item[0]
                if text :
                    text += " "
                text += item[2].strip()
                end = item[1]
                break # iba 1 iteracia
            self._b_x = (start, end, text)

        return True

    def process(self, small_window = 0.3, large_window = 0.7) :
        diff = self._a_x[0] - self._b_x[0]
        if abs(diff) > large_window :
            if self._a_x[0] < self._b_x[0] :
                self._a_next = True
                self._b_next = False
                return (self._a_x[2], "")
            else :
                self._a_next = False
                self._b_next = True
                return ("", self._b_x[2])

        # Try join at ',' if string length is too off
        a_text = self._a_x[2]
        b_text = self._b_x[2]
        balance = self._a_x[2].count(',') - self._b_x[2].count(',')
        len_diff = (len(a_text) - len(b_text))/min(len(a_text), len(b_text))
        while abs(len_diff) > 0.7 and balance != 0 :
            if balance < 0 and len_diff < 0 :
                self._a_next = True
                self._b_next = False
                if not self._make_iter() :
                    break
                a_text += " " + self._a_x[2]
                balance += self._a_x[2].count(',') + 1

            elif balance > 0 and len_diff > 0 :
                self._a_next = False
                self._b_next = True
                if not self._make_iter() :
                    break
                b_text += " " + self._b_x[2]
                balance -= self._b_x[2].count(',') + 1
            
            else :
                break

            len_diff = (len(a_text) - len(b_text))/min(len(a_text), len(b_text))

        self._a_next = self._b_next = True
        if abs(diff) <= small_window :
            return (a_text, b_text)
        else :
            return (a_text, b_text)

    def match(self) :
        self._a_it = iter(self._a_speeches)
        self._b_it = iter(self._b_speeches)
        self._a_next = True
        self._b_next = True
        mixed = []

        while self._a_it is not None and self._b_it is not None :
            if self._make_iter() :
                res = self.process()
                mixed.append(res)
            else :
                if self._a_x is not None :
                    mixed.append((self._a_x[2], ""))
                if self._b_x is not None :
                    mixed.append(("", self._b_x[2]))
                break

        while self._a_it is not None :
            mixed.append((self._a_x[2], ""))
            try :
                self._a_x = next(self._a_it)
            except StopIteration :
                self._a_it = None

        while self._b_it is not None :
            mixed.append(("", self._b_x[2]))
            try :
                self._b_x = next(self._b_it)
            except StopIteration :
                self._b_it = None

        return mixed
    
parser = argparse.ArgumentParser(description="Search, download and match two subtitles. Intented to use on eng-cze basis but might work for other languages too.")
parser.add_argument('url', help="URL link to reference subtitles. Use - to search for reference subtitles with other options.")
parser.add_argument('-i', help="IMDB identificator of reference subtitles (e.g. 0066921).")
parser.add_argument('-a', help="Movie hash of reference subtitles, must be used with -s.")
parser.add_argument('-l', help="Language of reference subtitles (e.g. eng, cze, slo, ...). Default: cze.", default="cze")
parser.add_argument('-s', help="Movie size in bytes of reference subtitles, must be used with -a.", default=0, type=int)
parser.add_argument('-t', help="Language of matching subtitles. Default: eng.", default="eng")

args = parser.parse_args(sys.argv[1:])
os_com = opensubs.OpenSubtitles("OS Test User Agent")

# Make some argument checks
if (args.a is not None and args.a != "") or (args.s is not None and args.s > 0):
    if (args.a is None or args.a == "") or (args.s is None or args.s <= 0):
        sys.stderr.write("Both movie hash and size must be suppplied.")
        os_com.log_out()
        sys.exit(1)

# Search for refence subtitles
need_info_fill = False
if(args.url and args.url != "-"):
    ref_links = os_com.search_subtitles(url = args.url)
else:
    try:
       descriptor = opensubs.OpenSubDescriptor(imdbid = args.i, movie_hash = args.a, movie_byte_size = args.s, language = args.l)
    except ValueError :
        sys.stderr.write("At least one option for identifying reference subtitles must be present.")
        os_com.log_out()
        sys.exit(1)

    ref_links = os_com.search_subtitles(descriptor)
    need_info_fill = True

if not ref_links :
    sys.stderr.write("Couldn't find reference subtitles.")
    os_com.log_out()
    sys.exit(1)

# Choose first in list, no constraints here
ref_link = ref_links[0]
if(need_info_fill) : 
    os_com.get_info_from_xml([ref_link])

# Search for matching subtitles
# Try searching whit Imdb ID
descriptor = opensubs.OpenSubDescriptor(imdbid=ref_link['IDMovieImdb'], language = args.t)
match_links = os_com.search_subtitles(descriptor)

if not match_links :
    # If no results, try fulltext movie name query
    descriptor = opensubs.OpenSubDescriptor(movie_name=ref_link['MovieName'], language = args.t)
    match_links = os_com.search_subtitles(descriptor)
    if match_links is False :
        # Couldn't find subtitles
        sys.stderr.write("Couldn't find adequate matching subtitles.")
        os_com.log_out()
        sys.exit(1)

# Filter by CD number
match_cdfit_links = [x for x in match_links if x['SubSumCD'] == ref_link['SubSumCD']]
if match_cdfit_links :
    match_links = match_cdfit_links

# Try to fill missing info from XML (can be slow because of HTML requests)
os_com.get_info_from_xml(match_links)

# Subtitles with smallest difference in movie byte size are chosen as best match
best_match = min(match_links, key=lambda x: abs(float(x['MovieByteSize']) - float(ref_link['MovieByteSize'])))

# Download subtitles
ref_sub = os_com.download_subtitles([ref_link])[0]
match_sub = os_com.download_subtitles([best_match])[0]

# Parse to SubRip format
ref_rip = subrip.subrip(ref_sub, ref_link['SubFormat'])
match_rip = subrip.subrip(match_sub, best_match['SubFormat'])

# Save subtitles for later in SubRip format
ref_rip.save("Reference.srt")
match_rip.save("Match.srt")

# Run comparision algorithm that will match subtitles on reference 
match = SubMatch(match_rip, ref_rip).match()
with open("match.txt", 'w', encoding='utf8') as f:
    for x in match :
        f.write(x[0] + "\t" + x[1] + "\n")
    
os_com.log_out()
sys.exit(0)