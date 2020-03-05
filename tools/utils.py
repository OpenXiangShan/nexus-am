from pathlib import Path
import re, shutil

AM_HOME    = (Path(__file__) / '../..').resolve()
EXPORT_DIR = AM_HOME / 'tools' / 'export'

def convert(r):
  return r.replace('.', r'\.').replace('*', r'[^/]*')

def list_filter(path, xs):
  for x in xs:
    if re.search(convert(x), path):
      return True
  return False

def export(white_list, black_list):
  def files():
    for abspath in AM_HOME.rglob('*'):
      if abspath.is_file():
        path = abspath.relative_to(AM_HOME)
        path_str = '/' + str(path)
        white = list_filter(path_str, white_list)
        black = list_filter(path_str, black_list)
        if white and not black:
          print('COPY', path)
          yield abspath, path
   
  try:
    shutil.rmtree(EXPORT_DIR)
  except:
    pass
  
  for abspath, relpath in files():
    src = abspath
    dst = EXPORT_DIR / relpath
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(src, dst)
 