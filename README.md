# page-fault-replacement-algo.-simulation

## reference string:
**random:** 透過隨機產生一範圍為[1, 800]之隨機起始page number(假設為a)和一continuous numbers(假設為[1, 25]範圍中隨機出一數b)，來產生一段reference string(範圍為a ~ a+b-1)透過反覆執行來產生出200000筆的reference string。

**Locality:** 首先決定出function數量(程式中設為100)，來表示該process當中呼叫了100個fucntions，每一個function都有自己的一段reference string(程式中每一function之reference string length為[1900, 2100]中的隨機數字)。產生一個function之reference string之方法為挑選一隨機數(範圍為[1, 800]，假設為a)，在選擇出該function可參考到的subset of string(範圍可為[1/15, 1/25]中一數乘上800，程式中為800/15)，故該function可產生之reference string為[a, (800/15)-1]之隨機數。

**我的reference string(continuity):** 方法機乎同上Locality，差別在於function可產生之reference string為[a, (800/7)-1]之連續數(a, a+1, a+2, … , (800/7)-1)。目的在於模擬function可能執行column-major的陣列存取，並執行寫入之I/O動作。

## algorithms:
**FIFO:** 透過先進先出的方式來挑選victim page，但可能出現Belady’s anomaly現象，也就是process分配的frames增加後，page fault發生了不減反升的結果。

**ARB:** 透過記錄reference bit的歷史紀錄，來比較其bit stream的大小，並將frames中擁有最小bit stream值的page當作victim page置換出。具體流程為每個週期(程式中將每20次的參考作為一個週期)會將frames中每個page之bit stream右移一個位元，再將該page之reference bit複製到其bit stream中的最高位元後將reference bit值設為0。

**ESC:** 將原本的second chance演算法再考慮到dirty bit以(reference bit, dirty bit)的形式來比較其優先序大小，被挑選成victim page的優先順序為(0, 0)、(0, 1)、(1, 0)、(1, 1)。如此除了考慮參考歷史之外還可以考量到page被修改過的歷史，進一步改善disk writes的次數。

**我的algorithm(Additional Dirty Bit):** 透過改善ARB的方式，將bit stream的考量點改變成dirty bit，因此再如果有特定幾個常被修改的pages出現時，可以大幅降低disk writes的次數。

## 比較
此程式會比較在frame size為20、40、60、80、100時各演算法在不同reference string下的page fault、interrupt、disk write、cost。

**interrupt:** 中斷給OS執行的動作(不包含發生page fault本身)。
**disk write:** 當曾經被寫入的page被置換出frame時發生。
**cost:** interrupt次數 + disk write次數。
