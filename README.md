reg-m
=====

Реалізація [машини з натуральнозначними регістрами](http://uk.wikipedia.org/wiki/%D0%9C%D0%B0%D1%88%D0%B8%D0%BD%D0%B0_%D0%B7_%D0%BD%D0%B0%D1%82%D1%83%D1%80%D0%B0%D0%BB%D1%8C%D0%BD%D0%BE%D0%B7%D0%BD%D0%B0%D1%87%D0%BD%D0%B8%D0%BC%D0%B8_%D1%80%D0%B5%D0%B3%D1%96%D1%81%D1%82%D1%80%D0%B0%D0%BC%D0%B8 "машини з натуральнозначними регістрами") на С++.  
Реалізовано всі передбачені даною мат. моделлю команди, в тому числі ініціалізація регістрів.  
Як і будь-яка практична реалізація, дана реалізація накладає обмеження на максимальну кількість регістрів, що можуть бути задіяні в програмі, а також на максимальне значення самих регістрів. Дані максимальні значення варіюються в залежності від типу машини та ОС, на яких виконується інтерпритатор.

#Синтаксис
###Коментарі:
Дозволено однорядкові коментарі, що можуть бути оформлені будь-яким чином, але обов’язково повинні слідувати одразу після закриваючої команду дужки, або, якщо це інструкція ініціалізації регістру, - одразу після числа. Коментарі інтерпритатором ігноруються.

#####Приклад:
`R1 = 2 все, що знаходиться після 2 - коментар.`  
`Z(3) обнулення 3-го регістру`

###Ініціалізація регістрів
`R<n> = m`,  
де n - натуральне число - ініціювати регістр з номером `n` значенням `m`.

#####Приклад:  
`R0 = 123; Ініціювати 0-й регістр значенням 123`


Всі регістри повинні бути ініційовані перед блоком з інструкціями. Ініціалізація регістрів після або безпосередньо в блоці інструкцій не передбачена даним інтерпритатором, і може приводити до непередбачуваних наслідків.
    
###Інструкції для виконання
0. `Z(n)` — обнулити регістр з номером `n` (R[n]:=0).
0. `S(n)` — інкремент (збільшення на одиницю) `n`-го регістра (R[n]:=R[n]+1).
0. `T(m, n)` — копіювати вміст регістру `m` в `n` (R[n]:=R[m]).
0. `J(m, n, q)` — якщо `R[m]=R[n]`, то наступною виконувати команду з номером `q`, інакше — наступну за списком.

#Приклад МНР-програми для даного інтерпритатора
Мета: створити програму додавання 2х чисел. Нехай це будуть числа 99 та 900. 

`r0=99`  
`r1=900` 

0. `j(1,2,5)`  
0. `s(0)`  
0. `s(2)`  
0. `j(0,0,1)`

#Ліцензія
Public domain.

#Інше
Файл .pro - файл проекту для Qt Creator.