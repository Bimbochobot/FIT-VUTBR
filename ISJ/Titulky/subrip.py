import re

class subrip :
    def __init__(self, text_data, format) :
        self.items = []
        if format == "srt" :
            self._parse(text_data)
        elif format == "sub" :
            self._conv_microdvd(text_data)
        else :
            # Try to guess format
            if re.match('^\d+\:\d+\:\d+,\d+ --> \d+\:\d+\:\d+,\d+$', text_data) is not None:
                self._parse(text_data)
            elif  re.match('^\{\d+\}\{\d+\}$', text_data) is not None:
                self._conv_microdvd(text_data)
            else :
                raise ValueError("Unsported subtitle format")

    def _parse(self, text_data) :
        def deformat_time(time): 
            tokens = time.split(":")
            return int(tokens[0])*3600.0 + int(tokens[1])*60.0 + float(tokens[2].replace(',','.'))

        sub_split = [x for x in re.findall('[^\r\n]+', text_data) if x] # Remove empty strings
        if sub_split and not sub_split[0].isdigit() : sub_split[0] = '1' # Try to repair broken subtitle
        text = ""
        new_item = False
        for i, x in enumerate(sub_split) :
            if new_item :
                new_item = False
                m = re.match('^(\d+\:\d+\:\d+,\d+) --> (\d+\:\d+\:\d+,\d+)$', x)
                if m is not None :
                    if text: 
                        self.items.append((start, end, text))
                    groups = m.groups()
                    (start, end, text) = (deformat_time(groups[0]), deformat_time(groups[1]), "")
                    continue
                else :
                    text += (sub_split[i-1] + " ")

            if x.isdigit() :
                new_item = True
            else :
                text += x + " "

        if text: 
            self.items.append((start, end, text))
        else :
            # Error?
            pass

    def _conv_microdvd(self, text_data, fps = 23.98) :
        sub_split = filter(bool,re.findall('[^\r\n]+', text_data)) # Remove empty strings
        for x in sub_split :
            m = re.match('^\{(\d+)\}\{(\d+)\}(.*)$', x)
            if m is not None :
                groups = m.groups()
                if groups[2] : self.items.append((int(groups[0])/fps, int(groups[1])/fps, groups[2].replace('|',' ')))

    # TODO : move to SubMatch
    def make_speeches(self) :
        speeches = []
        for x in self.items :
            arr=[x for x in re.split('(\.{1,3}|\?|!) ?', x[2]) if x]
            secs_per_char = (x[1]-x[0]) / len(x[2])
            start = x[0]
            while arr :
                text = ''.join(arr[:2])
                duration = len(text)*secs_per_char
                if re.match('.*\w+', text) is not None :
                    speeches.append((start, start+duration, text))
                del arr[:2]
                start += duration

        return speeches

    def save(self, filename) :
        def format_time(time):
            hours = int(time / 3600)
            minutes = int(time % 3600 / 60)
            seconds = int(time % 3600 % 60)
            millis = int(round(1000 * (time - int(time))))

            return "{:02}:{:02}:{:02},{:03}".format(hours, minutes, seconds, millis)

        with open(filename, "w", encoding='utf8') as f :
            for i, x in enumerate(self.items) :
                f.write("{0}\n{1} --> {2}\n{3}\n\n".format(i+1, format_time(x[0]), format_time(x[1]), x[2]))