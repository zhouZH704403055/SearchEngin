# coding:utf-8
'''
使用Python对所有的HTML文档进行预处理
目的：
    1.把所有的文档内容整合到一个文件中，方便后续Cpp进行处理
    2.把所有的HTML进行去标签
得到一个raw_input文件
文件格式：
    1.每行对应一个HTML文件
    2.每行中包含部分
        a)jump_url   //跳转URL  通过URL前缀+当前目录结构
        b)title    //标题   解析HTML，里面的title标签中
        c)content   //正文   对HTML内容进行去标签，也需要把所有换行去掉
'''
import os
import re
from bs4 import BeautifulSoup
import sys
reload(sys)
sys.setdefaultencoding('utf8')

input_path='../data/input/'
output_path='../data/tmp/raw_input'
url_prefix='https://www.boost.org/doc/libs/1_69_0/doc/'

def filter_tags(htmlstr):
    #先过滤CDATA
    re_cdata=re.compile('//<!\[CDATA\[[^>]*//\]\]>',re.I) #匹配CDATA
    re_script=re.compile('<\s*script[^>]*>[^<]*<\s*/\s*script\s*>',re.I)#Script
    re_style=re.compile('<\s*style[^>]*>[^<]*<\s*/\s*style\s*>',re.I)#style
    re_br=re.compile('<br\s*?/?>')  #处理换行
    re_h=re.compile('</?\w+[^>]*>') #HTML标签
    re_comment=re.compile('<!--[^>]*-->')   #HTML注释
    s=re_cdata.sub('',htmlstr)  #去掉CDATA
    s=re_script.sub('',s)   #去掉SCRIPT
    s=re_style.sub('',s)    #去掉style
    #把 br替换成空格
    s=re_br.sub(' ',s)     #将br转换为换行
    s=re_h.sub('',s)    #去掉HTML 标签
    s=re_comment.sub('',s)  #去掉HTML注释
    #去掉多余的空行
    blank_line=re.compile('\n+')
    #将多个换行替换成一个空格
    s=blank_line.sub(' ',s)
    s=replaceCharEntity(s)  #替换实体
    return s

#替换常用HTML字符实体.
#使用正常的字符替换HTML中特殊的字符实体.
#你可以添加新的实体字符到CHAR_ENTITIES中,处理更多HTML字符实体.
#@param htmlstr HTML字符串.
def replaceCharEntity(htmlstr):
    CHAR_ENTITIES={'nbsp':' ','160':' ',
                   'lt':'<','60':'<',
                   'gt':'>','62':'>',
                   'amp':'&','38':'&',
                   'quot':'"','34':'"',}

    re_charEntity=re.compile(r'&#?(?P<name>\w+);')
    sz=re_charEntity.search(htmlstr)
    while sz:
        entity=sz.group()#entity全称，如&gt;
        key=sz.group('name')#去除&;后entity,如&gt;为gt
        try:
            htmlstr=re_charEntity.sub(CHAR_ENTITIES[key],htmlstr,1)
            sz=re_charEntity.search(htmlstr)
        except KeyError:
            #以空串代替
            htmlstr=re_charEntity.sub('',htmlstr,1)
            sz=re_charEntity.search(htmlstr)
    return htmlstr

def enum_files(intput_path):
    '''
    获取input_path中所有HTML文件的路径
    '''
    file_list = []
    for basedir,dirnames,filenames in os.walk(input_path):
        for filename in filenames:
            if os.path.splitext(filename)[-1] != '.html':
                continue
            file_list.append(basedir + '/' + filename)
    return file_list

def parse_url(path):
    '''
    jump_url 格式形如: https://www.boost.org/doc/libs/1_69_0/doc/html/array.html
    path 格式形如: ../data/input/html/about.html
    url_prefix 的格式形如: https://www.boost.org/doc/libs/1_69_0/doc/
    '''
    return url_prefix + path[len(input_path):]

def parse_title(html):
    soup = BeautifulSoup(html,'html.parser')
    return soup.find('title').string

def parse_content(html):
    '''
    1.对正文进行去标签
    2.去除正文中的换行
    '''
    return filter_tags(html)


def parse_file(path):
    '''
    解析HTML文件，获取jump_url、tile、content
    返回一个三元组
    '''
    html = open(path).read()
    return parse_url(path),parse_title(html),parse_content(html)

def write_result(result,output_fd):
    '''
    把结果写入文件中
    '''
    output_fd.write(result[0] + '\3' + result[1] + '\3' + result[2] + '\n')

def run():
    '''整个预处理动作的入口函数'''
    #1.遍历目录下的所有HTML文件
    #file_lsit是一个列表，里面的每个元素都是一个文件路径
    file_list = enum_files(input_path)
    with open(output_path,'w') as output_fd:
        #2.针对每一个HTML文件，进行解析，获取jump_url、title、content
        for f in file_list:
            #parse_file返回结果是是一个三元组（jump_url、title、content）
            result = parse_file(f)
            #3.把结果写入输出文件
            if result[0] and result[1] and result [2]:
                write_result(result,output_fd)

def test1():
    file_list = enum_files(input_path)
    print file_list

def test2():
    file_list = enum_files(input_path)
    for f in file_list:
        print parse_file(f)

if __name__ == '__main__':
    run()
    #test1()
    #test2()









