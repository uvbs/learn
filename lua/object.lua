t = 
{
    age = 27,
    add = function(self, n)
        self.age = self.age + n
    end
}

print (t.age)
t:add(10)
print(t.age)
t.add(t, 10)
print(t.age)
