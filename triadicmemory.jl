
using Random

N = 1000
P = 10
temp_vec = zeros(Int64,P)
temp_vec_N =zeros(N)

function binarize(x,N,P)
    global temp_vec
    temp_vec .= zero(Int16)
    sorted = sort(x)
    rankedmax = sorted[ N - P + 1]
    if rankedmax == 0
		rankedmax = 1
    end

	j=1
	for i = 1:N
        if x[i] >= rankedmax
            global temp_vec
            # if (j > P)
            #     break
            # end
            temp_vec[j] = i
            #println("$i $(temp_vec[j])")
            j+=1
        end
    end
    return temp_vec
end


function insert(mem, xvec,yvec,zvec)
    for x in xvec
        for y in yvec
            for z in zvec
                mem[x,y,z] += 1
            end
        end
    end
end 

function insert(mem,cache, xvec,yvec,zvec)
    for x in xvec
        for y in yvec
            cache[x,y] = 1
            for z in zvec
                mem[x,y,z] += 1
            end
        end
    end
end 
# given x,y find z

function query(mem,xvec,yvec,zvec)
    temp_vec_N = zeros(Int16,N)
    
    if ismissing(zvec)
        for x in xvec
            @simd  for y in yvec
                @views temp_vec_N += mem[x,y,:]                
                #temp_vec_N += mem[x,y,:]
            end
        end
    elseif  ismissing(xvec)
        for y in yvec
            @simd  for z in zvec
                @views temp_vec_N += mem[:,y,z]                
                #temp_vec_N += mem[x,y,:]
            end
        end
    elseif ismissing(yvec)
        for x in xvec
            for z in zvec
                @views temp_vec_N += mem[x,:,z]                
                #temp_vec_N += mem[x,y,:]
            end
        end
    end
    
    z = binarize(temp_vec_N,N,P)
    return z
end 

function query_z2(mem,cache,xvec,yvec)
    temp_vec_N = zeros(Int16,N)
    #@time 
    for x in xvec, y in yvec
        if cache[x,y] == 1
            temp_vec_N += zeros(N) #mem[x,y,:]
        end
    end
    #@time 
    z = binarize(temp_vec_N,N,P)
    return z
end 
# now do an insertion and query test 

function test()

    CYCLES = 10000
    println("inserting $CYCLES vectors")
    w = @elapsed begin
        global CYCLES 
        mem = zeros(Int8,N,N,N);
        cache = zeros(Int8,N,N);
        # prepare test vectors
        x_input = zeros(Int64,CYCLES,P)
        y_input = zeros(Int64,CYCLES,P)
        z_input = zeros(Int64,CYCLES,P)

        for i in 1:CYCLES
            x_input[i,:] = sort(ceil.(Int,rand(P)*N))
            y_input[i,:] = sort(ceil.(Int,rand(P)*N))
            z_input[i,:] = sort(ceil.(Int,rand(P)*N))
        end 
    
        # check that the vectors dont have identical numbers 

        while true 
            redo = false
            for i in 1:CYCLES
                for k in 1:P-1
                    if x_input[i,k] == x_input[i,k+1]
                        x_input[i,k+1] = ceil(Int,rand()*N)
                        x_input[i,:] = sort(x_input[i,:])
                        redo = true
                        break
                    end
                    if y_input[i,k] == y_input[i,k+1]
                        y_input[i,k+1] = ceil(Int,rand()*N)
                        y_input[i,:] = sort(y_input[i,:])
                        redo = true
                        break
                    end
                    if z_input[i,k] == z_input[i,k+1]
                        z_input[i,k+1] = ceil(Int,rand()*N)
                        z_input[i,:] = sort(z_input[i,:])
                        redo = true
                        break
                    end
                end
            end
            redo || break
        end
    end
    speed = round(CYCLES/w, digits=2)
    println("preparing $CYCLES vectors took $w seconds: $speed per sec")
    w = @elapsed  for i in 1:CYCLES
        x = x_input[i,:]
        y = y_input[i,:]
        z = z_input[i,:]
        #insert(mem,x,y,z)
        insert(mem,cache, x,y,z)
    end
    speed = round(CYCLES/w, digits=2)
    println("inserting $CYCLES vectors took $w seconds: $speed per sec")
    errors = 0
    @time w = @elapsed for i in 1:CYCLES
        x = x_input[i,:]
        y = y_input[i,:]
        z = z_input[i,:]
        qz = query(mem,x,y, missing)
         if  qz != z
            #println("($i)\nOrig z: $z\nQuer z: $qz")
            errors += 1
        end
    end
    speed = round(CYCLES/w, digits=2)
    println("querying z $CYCLES vectors took $w seconds: $speed per sec, $errors errors")

    errors = 0
    @time w = @elapsed for i in 1:CYCLES
        x = x_input[i,:]
        y = y_input[i,:]
        z = z_input[i,:]
        qy = query(mem,x,missing,z)
         if  qy != y
            errors += 1
        end
    end
    speed = round(CYCLES/w, digits=2)
    println("querying y $CYCLES vectors took $w seconds: $speed per sec, $errors errors")

    errors = 0
    @time w = @elapsed for i in 1:CYCLES
        x = x_input[i,:]
        y = y_input[i,:]
        z = z_input[i,:]
        qx = query(mem,missing,y,z)
         if  qx != x
             errors += 1
        end
    end
    speed = round(CYCLES/w, digits=2)
    println("querying x $CYCLES vectors took $w seconds: $speed per sec, $errors errors")

end 


test()