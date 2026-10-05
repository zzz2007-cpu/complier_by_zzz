from pathlib import Path
import random,re,subprocess,tempfile,json,resource
resource.setrlimit(resource.RLIMIT_CORE,(0,0))
r=random.Random(20261004)
root=Path(tempfile.mkdtemp(prefix='parser-deep-audit-'))
exe='/tmp/compiler-parser-validation-build/Compiler'
results=[]
def run(name,source,errors='',tree=None,group='normal'):
 p=root/name;p.mkdir();(p/'testfile.txt').write_text(source)
 try:
  q=subprocess.run([exe],cwd=p,capture_output=True,text=True,timeout=5)
  actual=(p/'parser.txt').read_text() if (p/'parser.txt').exists() else ''
  err=(p/'error.txt').read_text() if (p/'error.txt').exists() else None
  ok=q.returncode==0 and err==errors and actual.endswith('<CompUnit>\n') and (tree is None or actual==tree)
  info=dict(name=name,group=group,passed=ok,exit=q.returncode,expected_errors=errors,actual_errors=err,exact_tree=tree is not None,tree_matches=tree is None or actual==tree,stderr=q.stderr)
  (p/'stderr.txt').write_text(q.stderr)
 except subprocess.TimeoutExpired:
  actual='';info=dict(name=name,group=group,passed=False,reason='timeout')
 if tree is not None:(p/'expected_parser.txt').write_text(tree)
 (p/'expected_error.txt').write_text(errors)
 results.append(info)
 return actual
# Independent grammar-tree generator: serialize each constructed tree in postorder.
def T(kind,word):return ('token',kind,word)
def N(kind,*children):return (kind,list(children))
def emit(n):
 if n[0]=='token':return n[1]+' '+n[2]+'\n'
 return ''.join(emit(c) for c in n[1])+('<'+n[0]+'>\n' if n[0] not in ('BType','Decl','BlockItem') else '')
def source(n):
 if n[0]=='token':return n[2]
 return ' '.join(source(c) for c in n[1])
def number(word='0',kind='INTCON'):return N('Number',T(kind,word))
def unary(d,cond=False,char=False):
 choices=['number'] if d==0 else ['number','paren','cast','sign','call']
 if cond and not char:choices+=['not']
 choice=r.choice(choices)
 if char and choice in ('sign','call','not'):choice='number'
 if choice=='number':
  val=number(r.choice(["'a'","' '","'\\n'","'\\t'","'\\r'","'\\0'","'\\\\'","'\\\''"]),'CHARCON') if char else number(str(r.randrange(1,10)))
  return N('UnaryExp',N('PrimaryExp',val))
 if choice=='paren':return N('UnaryExp',N('PrimaryExp',T('LPARENT','('),exp(d-1,char),T('RPARENT',')')))
 if choice=='cast':return N('UnaryExp',T('LPARENT','('),N('BType',T('CHARTK' if char else 'INTTK','char' if char else 'int')),T('RPARENT',')'),unary(d-1,False,not char))
 if choice=='call':return N('UnaryExp',T('IDENFR','get_int'),T('LPARENT','('),T('RPARENT',')'))
 op=('NOT','!') if choice=='not' else r.choice([('PLUS','+'),('MINU','-')])
 return N('UnaryExp',N('UnaryOp',T(*op)),unary(max(d-1,0),cond,char))
def chain(kind,child,ops,count):
 n=N(kind,child())
 for _ in range(count):n=N(kind,n,T(*r.choice(ops)),child())
 return n
def mul(d,char=False,cond=False):
 # Use multiplication only: avoid accidentally generating undefined division by zero.
 return chain('MulExp',lambda:unary(d,cond,char),[('MULT','*')],0 if char else r.randrange(2))
def add(d,char=False,cond=False):return chain('AddExp',lambda:mul(d,char,cond),[('PLUS','+'),('MINU','-')],0 if char else r.randrange(3))
def exp(d,char=False):return N('Exp',add(d,char))
def cond():
 rel=lambda:chain('RelExp',lambda:add(1,False,True),[('LSS','<'),('GRE','>'),('LEQ','<='),('GEQ','>=')],r.randrange(2))
 eq=lambda:chain('EqExp',rel,[('EQL','=='),('NEQ','!=')],r.randrange(2))
 land=lambda:chain('LAndExp',eq,[('AND','&&')],r.randrange(2))
 return N('Cond',chain('LOrExp',land,[('OR','||')],r.randrange(2)))
def ret(e):return N('Stmt',T('RETURNTK','return'),e,T('SEMICN',';'))
def program(*stmts):return N('CompUnit',N('MainFuncDef',T('INTTK','int'),T('MAINTK','main'),T('LPARENT','('),T('RPARENT',')'),N('Block',T('LBRACE','{'),*stmts,T('RBRACE','}'))))
for i in range(160):
 n=program(ret(exp(2)));run(f'exp_{i:03}',source(n),'',emit(n),'generated_exact')
for i in range(80):
 n=program(N('Stmt',T('IFTK','if'),T('LPARENT','('),cond(),T('RPARENT',')'),ret(N('Exp',N('AddExp',N('MulExp',N('UnaryExp',N('PrimaryExp',number('1')))))))),ret(N('Exp',N('AddExp',N('MulExp',N('UnaryExp',N('PrimaryExp',number())))))))
 run(f'cond_{i:03}',source(n),'',emit(n),'generated_exact')
for i in range(20):
 e=N('Exp',N('AddExp',N('MulExp',N('UnaryExp',T('LPARENT','('),N('BType',T('INTTK','int')),T('RPARENT',')'),unary(2,False,True)))))
 n=program(ret(e));run(f'char_{i:03}',source(n),'',emit(n),'generated_exact')
fixture='''const int n=3, k=1, z=0;
const char msg[3]="ok";
int data[3]={0,1,2};
int sum(int a[], int x, int y) {
    return a[x];
}
char identity(char c) {
    return c;
}
void empty() {
    return;
}
int main() {
    static int x=0;
    char text[3]="hi";
    empty();
    data[x]=sum(data,1,2);
    x=(int)identity('a');
    while(x>0 && x!=3) {
        x=x-1;
        if(x==2)continue;
        else break;
    }
    switch(x) {
        case 1: x=2; break;
        default: x=0; break;
    }
    printf("%d%c%s", x, identity('a'), text);
    return (x+1);
}
'''
base=run('fixture',fixture,group='fixture')
pattern=r'"[^"\n]*"|\'(?:\\.|[^\'\\])\'|[A-Za-z_]\w*|\d+|&&|\|\||==|!=|<=|>=|\S'
tokens=list(re.finditer(pattern,fixture));candidates=[]
for i,t in enumerate(tokens):
 if t.group() not in (';',')',']'):continue
 # Only remove delimiters at unambiguous positions; closing nested parens can shift tree boundaries.
 if t.group()==')' and i+1<len(tokens) and tokens[i+1].group() in ('+', '-', '*', '/', '%',')',','):continue
 previous=tokens[i-1];line=fixture[:previous.start()].count('\n')+1
 code={';':'i',')':'j',']':'k'}[t.group()]
 candidates.append((t.start(),line,code))
 run(f'delete_{t.start()}',fixture[:t.start()]+' '+fixture[t.end():],f'{line} {code}\n',base,'deletion')
# Separate independent lines for combined errors.
for j in range(40):
 picked=[];lines=set()
 for item in r.sample(candidates,len(candidates)):
  if item[1] not in lines:picked.append(item);lines.add(item[1])
  if len(picked)==3:break
 changed=fixture
 for pos,_,_ in sorted(picked,reverse=True):changed=changed[:pos]+' '+changed[pos+1:]
 expected=''.join(f'{line} {code}\n' for _,line,code in sorted(picked,key=lambda x:x[1]))
 run(f'combined_{j}',changed,expected,base,'combined')
clean='int main(){\nreturn 0;\n}\n';clean_tree=run('comment_baseline',clean,group='comments')
comments={
'line_before':'// hello\n'+clean,
'block_before':'/* hello */\n'+clean,
'line_body':'int main(){\n// hello\nreturn 0;\n}',
'block_body':'int main(){\n/* hello */\nreturn 0;\n}',
'block_multiline':'int main(){\n/* hello\nworld */\nreturn 0;\n}',
'between_tokens':'int/**/main(){return/**/0;}',
'line_after_return':'int main(){\nreturn 0; // hello\n}',
'block_fake_tokens':'int main(){/* ] ) & | \' " */return 0;}',
'line_at_eof':clean+'// trailing',
'block_at_eof':clean+'/* trailing */',
}
for name,s in comments.items():run(name,s,'',clean_tree,'comments')
run('comment_error_line','int main(){\n/* first\nsecond */\nreturn 0\n}\n','4 i\n',group='comments')
run('string_comment_markers','int main(){printf("// /* */");return 0;}',group='lexical')
run('and_or_errors','int main(){\nif(1 & 2)return 1;\nif(1 | 2)return 2;\nreturn 0;}','2 a\n3 a\n',group='lexical')
run('crlf','int main(){\r\nreturn 0\r\n}\r\n','2 i\n',group='lexical')
(root/'results.json').write_text(json.dumps(results,ensure_ascii=False,indent=2))
print('ARTIFACTS',root)
for group in sorted(set(x['group'] for x in results)):
 items=[x for x in results if x['group']==group];print(group,sum(x['passed'] for x in items),'/',len(items))
for x in results:
 if not x['passed']:print('FAIL',x['name'],x.get('exit'),repr(x.get('actual_errors')), 'expected',repr(x.get('expected_errors')),'tree_matches',x.get('tree_matches'))
