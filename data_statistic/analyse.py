from os import listdir
from os.path import isfile, join

MAIL_PATH = 'mails'

mail_files = [f for f in listdir(MAIL_PATH) if isfile(join(MAIL_PATH, f))]

delim = ['\n', ' ', '!', '"', '#', '$', '%', '&', "'", '(', ')', '*', '+', ',', '-', '.', '/', ':', ';', '<', '=', '>', '?', '@', '[', '\\', ']', '^', '_', '`',  '{', '|', '}', '~']

def parse_file (file_name):
  file = open(join(MAIL_PATH, file_name), 'r')
  lines = file.readlines()
  file.close()

  subject = ' '.join(lines[3].split()[1:])
  content = lines[6]

  return (subject + content).lower()

def toSet (bag_of_words):
  for d in delim:
    bag_of_words = bag_of_words.replace(d, ' ')
  word_list = bag_of_words.split(' ')
  return set(word_list) - set([''])


output = open('stat.txt', 'a')

string_set = set()
for mail in mail_files:
  bag_of_words = parse_file(mail)
  set_of_words = toSet(bag_of_words)

  output.write(str(len(set_of_words)) + '\n')
  string_set |= set_of_words

output.close()

print(f"Total {len(string_set)} unique strings")