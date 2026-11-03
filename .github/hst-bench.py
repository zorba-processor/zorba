import glob,os,statistics,sys,time
zb,zh=sys.argv[1:3]
def gen():
    os.makedirs('/tmp/g',exist_ok=True)
    def lets(n,v):
        return ' '.join('let $%s%d := $%s%d + 1'%(v,i+1,v,i) for i in range(n))
    def wide(M,N,ret):
        return '(' + ',\n'.join('for $i%d in 1 to 2 let $a0 := $i%d %s return %s'%(m,m,lets(N,'a'),ret%N) for m in range(M)) + ')'
    fs='declare function local:i($x) { $x };\ndeclare variable $fs := (local:i#1, local:i#1);\n'
    ef='declare namespace an = "http://zorba.io/annotations";\ndeclare %an:assignable variable $g := 0;\n'
    funcs=''.join('declare function local:f%d($x) { for $y in 1 to 2 let $z := $x + %d return $z + $y };\n'%(i,i) for i in range(500))
    calls=',\n'.join('local:f%d(1)'%i for i in range(500))
    q={
     'compile_no_call_wide': funcs+'count((%s,\n%s))\n'%(calls,wide(300,30,'$a%d')),
     'compile_no_call_deep': 'declare variable $k as xs:integer external := 1;\nlet $a0 := $k %s return $a600\n'%lets(600,'a'),
     'compile_pure_call_wide': fs+funcs+'count((%s,\n%s))\n'%(calls,wide(300,30,'$fs[1]($a%d)')),
     'compile_pure_call_deep': fs+'declare variable $k as xs:integer external := 1;\nlet $a0 := $k %s return $fs[1]($a600)\n'%lets(600,'a'),
     'compile_effects_call_wide': ef+fs+funcs+'$g := 1;\ncount((%s,\n%s))\n'%(calls,wide(300,30,'$fs[1]($a%d)')),
     'compile_effects_call_deep': ef+fs+'declare variable $k as xs:integer external := 1;\n$g := 1;\nlet $a0 := $k %s return $fs[1]($a600)\n'%lets(600,'a'),
    }
    for n,t in q.items():
        open('/tmp/g/%s.xq'%n,'w').write(t)
    body='$fs[1]($g)'
    for i in range(150): body='(for $v%d in 1 to 1 return ($v%d, %s))'%(i,i,body)
    open('/tmp/g/compile_effects_nested.xq','w').write(ef+fs+'$g := 1;\ncount(%s)\n'%body)
gen()
qs=sorted(glob.glob('/tmp/q/effects_*.xq'))
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
