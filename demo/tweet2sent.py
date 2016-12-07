#!/path/to/python
# -*- coding: utf-8 -*-
import sys, re, os, unicodedata, time, htmlentitydefs, signal
import neologdn

pat_rep   = re.compile (u'\.\s*@')
pat_hash  = re.compile (u'#\S+')
pat_sp    = re.compile (u'(?:　|\t)+')
pat_rt    = re.compile (u'\s*(?:(?:RT|QT):?|<)\s*(?:(?:@[\w\d_]*|[\w\d_]+)\s*:?|\s*:)\s*')
pat_dot   = re.compile (u'(\d\.)([^\d])')
pat_quot  = re.compile (u'("|“|”)')
# pat_ret   = re.compile (u'\s*(?:(?:@[\w\d_]+|RT|QT|<)\s*:?\s*)*(.+?(?:$|(?:。|！|？|!|\?)(?!」|』|）|】|\]|\)|。|！|？|!|\?)|(?:w|W)(?![a-zA-Z1-9]|」|』|）|】|\]|\)|。|！|？|!|w|\?)))') # @? -> @
pat_ret   = re.compile (u'\s*(?:(?:@[\w\d_]+|RT|QT|<)\s*:?\s*)*(.+?(?:$|(?:。|！|？|!|\?|[^\w] |】|…|・・・)(?!」|』|）|】|\]|\)|。|！|？|!|\?)|(?:w|W)(?![a-zA-Z1-9]|」|』|）|】|\]|\)|。|！|？|!|w|\?)))') # @? -> @
pat_label = re.compile (u'^\s*(?:\[|\(|［|【|（).+?(?:\]|\)|］|】|）)')
pat_ja    = re.compile (u'[ぁ-んァ-ン]')
pat_url    = re.compile (u'https?://[\w\./#]+')

pat_name  = re.compile (u'@[0-9a-z_]{1,15}')

pat_href   = re.compile (u'&(#x?[0-9a-f]+|[a-z]+);', re.IGNORECASE)
pat_nref16 = re.compile (u'#x\d+', re.IGNORECASE)
pat_nref10 = re.compile (u'#\d+',  re.IGNORECASE)

def deref (m):
    name = m.group (1)
    try:
        if name in htmlentitydefs.name2codepoint.keys ():
            return unichr (htmlentitydefs.name2codepoint[name])
        elif pat_nref16.match (name):
            return unichr (int (u'0' + name[1:], 16))
        elif pat_nref10.match (name):
            return unichr (int (name[1:]))
    except:
        sys.stderr.write ("unregularized: %s\n" % m.group ())
    return m.group ()


def handle_text (tweet, output):
    sents = pat_ret.findall (tweet)
    if len (sents) > 0:
        try:
            for sent in sents:
                sent = sent.lstrip ().rstrip ()
                if sent:
                    m = pat_nouns.search (sent)
                    if m:
                        out = neologdn.normalize (sent).encode ('utf-8')
                        if out:
                          output += out + "\n"
        except:
            pass
    return output

def handler (signum, frame):
    sys.exit (0);

# signal.signal (signal.SIGPIPE, signal.SIG_DFL)
signal.signal (signal.SIGPIPE, handler)
# for line in sys.stdin:

pat_nouns = re.compile (u'(?:' + '|'.join (x.decode ('utf-8') for x in sys.argv[1].split (",")) + ')')

for line in iter (sys.stdin.readline, ""): # no buffering
    try:
        text = line[:-1].decode ('utf-8') # may fail
#        if not pat_ja.search (text):
#            continue
    except:
        continue
    if text == "EOF":
        sys.exit (0)
    text = pat_url.sub  ('URL', text)
    text = pat_url.sub  ('', text)
    text = pat_href.sub (deref, text)
    text = pat_sp.sub   (r' ', text) # 　->
    text = pat_quot.sub (r'',  text) # erase delete annoying quote
    text = pat_rep.sub  (r'@', text) # .@ -> @
    text = pat_hash.sub (r'',  text) # hash tags
    text = pat_dot.sub  (r'\1 \2', text)
    output = ""
    m = pat_rt.search (text)
    output = handle_text (text[:m.start ()] if m else text, output)
    sys.stdout.write (output if output else "empty\n")
    # sys.stdout.write (output.replace("\n", "") if output else "empty")
    sys.stdout.flush ()
