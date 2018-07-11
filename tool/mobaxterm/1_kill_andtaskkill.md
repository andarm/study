``` 

VAR=`ps|grep notepad|awk '{print $1}'`
main()
{
        cd /home/mobaxterm
        echo `ls`
        #echo `ps -W`
        VAR=`ps -W|grep notepad|awk '{print $1}'`
        for value in $VAR
        do
                echo /bin/kill -9 $value
                taskkill.exe /PID $value /T
        done
        echo "###runing ....###"
        echo $VAR
}
main
 
``` 
