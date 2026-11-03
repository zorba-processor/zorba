import glob,os,statistics,sys,time
zb,zh=sys.argv[1:3]
def gen():
    os.makedirs('/tmp/g',exist_ok=True)
    N=2000
    lets='\n'.join('let $a%d := $a%d + 1'%(i+1,i) for i in range(N))
    w=open('/tmp/g/compile_no_call.xq','w')
    w.write('declare variable $k as xs:integer external := 1;\n%s\nlet $a0 := $k\n%s\nreturn $a%d\n'%('\n'.join('declare function local:f%d($x) { $x + %d };'%(i,i) for i in range(500)),lets,N))
    w.close()
    w=open('/tmp/g/compile_pure_call.xq','w')
    w.write('declare variable $k as xs:integer external := 1;\ndeclare function local:i($x) { $x };\ndeclare variable $fs := (local:i#1, local:i#1);\nlet $a0 := $k\n%s\nreturn $fs[1]($a%d)\n'%(lets,N))
    w.close()
    w=open('/tmp/g/compile_effects_call.xq','w')
    w.write('declare namespace an = "http://zorba.io/annotations";\ndeclare %%an:assignable variable $g := 0;\ndeclare variable $k as xs:integer external := 1;\ndeclare function local:i($x) { $x };\ndeclare variable $fs := (local:i#1, local:i#1);\n$g := 1;\nlet $a0 := $k\n%s\nreturn $fs[1]($a%d)\n'%(lets,N))
    w.close()
    w=open('/tmp/g/compile_effects_nested.xq','w')
    body='$fs[1]($g)'
    for i in range(150): body='(for $v%d in 1 to 1 return ($v%d, %s))'%(i,i,body)
    w.write('declare namespace an = "http://zorba.io/annotations";\ndeclare %%an:assignable variable $g := 0;\ndeclare function local:i($x) { $x };\ndeclare variable $fs := (local:i#1, local:i#1);\n$g := 1;\ncount(%s)\n'%body)
    w.close()
gen()
qs=sorted(glob.glob('/tmp/q/*.xq'))+sorted(glob.glob('/tmp/g/*.xq'))
for q in qs:
    n=os.path.basename(q)[:-3]
    for which,z in (('base',zb),('head',zh)):
        ts=[];ms=[];v=''
        for r in range(5):
            pid=os.fork()
            if pid==0:
                os.dup2(os.open('/tmp/out.txt',os.O_WRONLY|os.O_CREAT|os.O_TRUNC),1)
                os.dup2(os.open('/tmp/err.txt',os.O_WRONLY|os.O_CREAT|os.O_TRUNC),2)
                os.execv('/usr/bin/timeout',['timeout','280',z,'-f','-q',q])
            t=time.time()
            _,st,ru=os.wait4(pid,0)
            ts.append(time.time()-t);ms.append(ru.ru_maxrss)
            v=open('/tmp/out.txt').read().strip().split(chr(10))[-1][-40:]
            if st!=0: v='EXIT %d %s'%(st,open('/tmp/err.txt').read()[-160:].replace(chr(10),' '))
        print('RES',n,which,'median %.3f s'%statistics.median(ts),'min %.3f max %.3f'%(min(ts),max(ts)),'rss %.0f MiB'%(max(ms)/1024),'value',v,flush=True)
