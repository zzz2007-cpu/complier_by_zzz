from pathlib import Path
import subprocess, tempfile, re, resource, json
resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
repo=Path('/home/zzz2007-cpu/projects/compiler')
exe='/tmp/compiler-parser-validation-build/Compiler'
out=Path(tempfile.mkdtemp(prefix='compiler-parser-results-'))
results=[]
def run(name,source,expected='',expected_tree=None):
 p=out/name;p.mkdir();(p/'testfile.txt').write_text(source)
 try:
  r=subprocess.run([exe],cwd=p,capture_output=True,text=True,timeout=5)
  err=(p/'error.txt').read_text() if (p/'error.txt').exists() else None
  tree=(p/'parser.txt').read_text() if (p/'parser.txt').exists() else ''
  ok=r.returncode==0 and err==expected and tree.endswith('<CompUnit>\n') and (expected_tree is None or tree==expected_tree)
  info=dict(name=name,passed=ok,exit=r.returncode,expected_errors=expected,actual_errors=err,stderr=r.stderr,exact_tree_checked=expected_tree is not None)
 except subprocess.TimeoutExpired:
  tree='';info=dict(name=name,passed=False,reason='timeout')
 results.append(info)
 return tree
text=(repo/'docs/实验要求/实验二语法分析.md').read_text()
g=re.findall(r'```\n(.*?)```',text.split('### 正确源程序样例')[1].split('### 错误源程序样例')[0],re.S)
b=re.findall(r'```\n(.*?)```',text.split('### 错误源程序样例')[1],re.S)
n=lambda s:s.replace('\xa0',' ').strip()+'\n'
run('course_correct',n(g[0]),expected_tree=n(g[1]))
run('course_errors',n(b[0]),n(b[1]))
# Each @ marks a delimiter to remove. Compare the recovered tree with the intact program's tree.
mutations={
'const_semicolon':'const int a=1@;\nint main(){return a;}',
'var_semicolon':'int main(){\nint a=1@;\nreturn a;}',
'static_semicolon':'int main(){\nstatic int a=1@;\nreturn a;}',
'assignment_semicolon':'int main(){int a;\na=1@;\nreturn 0;}',
'exp_semicolon':'int main(){int a=1;\na+1@;\nreturn 0;}',
'break_semicolon':'int main(){while(1){\nbreak@;\n}return 0;}',
'continue_semicolon':'int main(){while(1){\ncontinue@;\n}return 0;}',
'return_value_semicolon':'int main(){\nreturn 0@;\n}',
'return_empty_semicolon':'void f(){\nreturn@;\n}\nint main(){return 0;}',
'printf_semicolon':'int main(){\nprintf("hello")@;\nreturn 0;}',
'main_rparen':'int main(@) {return 0;}',
'func_empty_rparen':'void f(@) {return;}\nint main(){return 0;}',
'func_params_rparen':'int f(int x@) {return x;}\nint main(){return f(1);}',
'call_empty_rparen':'void f(){return;}\nint main(){\nf(@);\nreturn 0;}',
'call_params_rparen':'int f(int x){return x;}\nint main(){\nreturn f(1@);\n}',
'if_rparen':'int main(){\nif(1@){return 1;}\nreturn 0;}',
'while_rparen':'int main(){\nwhile(1@){break;}\nreturn 0;}',
'switch_rparen':'int main(){\nswitch(1@){case 1:break;}\nreturn 0;}',
'primary_rparen':'int main(){\nreturn (1+2@);\n}',
'cast_rparen':'int main(){\nreturn (int@)\'a\';\n}',
'printf_rparen':'int main(){\nprintf("hello"@);\nreturn 0;}',
'const_rbracket':'const int a[1@]={1};\nint main(){return a[0];}',
'var_rbracket':'int main(){\nint a[1@]={1};\nreturn a[0];}',
'param_rbracket':'int f(int a[@]){return a[0];}\nint main(){int a[1]={1};return f(a);}',
'lval_read_rbracket':'int main(){int a[1]={1};\nreturn a[0@];\n}',
'lval_assign_rbracket':'int main(){int a[1];\na[0@]=1;\nreturn 0;}',
'lval_expression_boundary':'int main(){int a[1]={0},b;\na[0@];\nb=1;return 0;}',
'nested_lval_rbracket':'int main(){int a[3]={0,1,2};\na[a[0]@]=1;\nreturn 0;}',
}
for name,marked in mutations.items():
 i=marked.index('@');symbol=marked[i+1];line=marked[:i].count('\n')+1
 original=marked.replace('@','');bad=marked[:i]+marked[i+2:]
 tree=run(name+'_intact',original)
 run(name+'_missing',bad,f'{line} '+{';':'i',')':'j',']':'k'}[symbol]+'\n',tree)
good={
'minimal':'int main(){return 0;}',
'declarations':'const int n=3,a[3]={1,2,3}; const char s[3]="ab"; int g[3]={}; int main(){static int x=1,y=2; char t[3]="hi"; int b[3]={1,2,3}; return x+y;}',
'functions':'void f(){return;} char c(char a){return a;} int sum(int a[],int n){return a[0]+n;} int main(){int a[1]={1};f();c(\'a\');return sum(a,2);}',
'operators':'int main(){int a=1,b=2; a=10-3-2*4/2%3; if(!a||a<b&&a<=b||a>b&&a>=b||a==b||a!=b)a=+a;else a=-a; return (int)(char)(a+b);}',
'loops_switch':'int main(){int x=0;while(x<3){x=x+1;if(x==1)continue;switch(x){case 2:{int y=0;y=y+1;}break;default:break;}if(x==2)break;}return 0;}',
'nested_index':'int main(){int a[3]={0,1,2};a[a[0]]=a[a[1]]; a[0]+1; ; {} return 0;}',
'dangling_else':'int main(){if(1)if(0)return 1;else return 2;return 0;}',
'empty_initializers':'const int a[2]={}; int main(){int b[2]={};switch(1){}return 0;}'
}
for name,src in good.items():run(name,src)
run('four_errors','void f( {\n    return\n}\nint main() {\n    int a[3];\n    f(;\n    a[0 = 1;\n    return 0;\n}\n','1 j\n2 i\n6 j\n7 k\n')
run('mixed_errors','int main(){\nint a[3;\nif(1 & 2){\nprintf("ok";\n}\nreturn 0\n}','2 k\n3 a\n4 j\n6 i\n')
(out/'results.json').write_text(json.dumps(results,ensure_ascii=False,indent=2))
print('Artifacts:',out)
print('Total:',len(results),'Passed:',sum(r['passed'] for r in results))
for r in results:
 if not r['passed']: print(json.dumps(r,ensure_ascii=False))
print('Delimiter recovery cases:',len(mutations))
print('Exact tree comparisons:',sum(r.get('exact_tree_checked',False) for r in results))
