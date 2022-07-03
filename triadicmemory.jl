
#module TriadicMemory

using Random

N = 1000
P = 10
temp_vec = zeros(Int64,P)
temp_vec_N =zeros(N)

function binarize(x,r,N,P)
    sorted = sort(x)
    rankedmax = sorted[ N - P + 1]
    if rankedmax == 0
		rankedmax = 1
    end

	j=1
	for i = 1:N
        if x[i] >= rankedmax
            if (j<= P) 
                r[j] = i
            else
                println("Warning: j>P - rankedmax is $rankedmax,  retrieved vector is probably wrong")
            end
            j+=1
        end
    end
end


# we insert into memory in two diffferent column based order:
# once for a sequencial access and once for z. We use the fact that the high nibble
# of the memory content is mostly unused and can use it to store the second ordering count
# idea taken from Scheme implementation of Roger Turner
# https://github.com/rogerturner/TriadicMemory/blob/main/triadicmemory.ss

function insert(mem, xvec,yvec,zvec)
    for z in zvec
        for y in yvec
            for x in xvec
                mem[x,y,z] += 1
            end
        end
    end
    for x in xvec
        for y in yvec
            for z in zvec
                mem[z,y,x] += 16
            end
        end
    end
end 


function query(mem,xvec,yvec,zvec)
    temp_vec_N = zeros(Int16,N)
    ret_vec = zeros(Int16,P)
    
    # notice that z sequencial access is better done using 
    # the x "like" locations - just looking at the high nibble
    # this allows us to speed up memory access since Julia is storing
    # sequece in column order see https://discourse.numenta.org/t/triadic-memory-a-fundamental-algorithm-for-cognitive-computing/9763/73
    # for more details..
    if ismissing(zvec)
        for x in xvec
            @simd  for y in yvec
                @views temp_vec_N += (mem[:,y,x] .>> 4)               
            end
        end
    elseif  ismissing(xvec)
        for y in yvec
            @simd  for z in zvec
                @views temp_vec_N += (mem[:,y,z] .& 0x0f)
            end
        end
    elseif ismissing(yvec)
        for x in xvec
            @simd for z in zvec
                @views temp_vec_N += (mem[x,:,z] .& 0x0f)                
            end
        end
    end
    
    binarize(temp_vec_N,ret_vec, N,P)
    return ret_vec
end 

# now do an insertion and query test 
function test()

    CYCLES = 10000
    println("inserting $CYCLES vectors")
    w = @elapsed begin
        global CYCLES 
        mem = zeros(UInt8,N,N,N);
        cache = zeros(UInt8,N,N);
        # prepare test vectors
        x_input = zeros(Int64,CYCLES,P)
        y_input = zeros(Int64,CYCLES,P)
        z_input = zeros(Int64,CYCLES,P)

        for i in 1:CYCLES
            # ceil will make sure that the smallest number is 1 
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
        insert(mem,x,y,z)
    end
    speed = round(CYCLES/w, digits=2)
    println("inserting $CYCLES vectors took $w seconds: $speed per sec")


    # Query Z given x,y
    ####################
    errors = 0
    @time w = @elapsed for i in 1:CYCLES
        x = x_input[i,:]
        y = y_input[i,:]
        z = z_input[i,:]
        qz = query(mem,x,y,missing)
         if  qz != z
            errors += 1
        end
    end
    speed = round(CYCLES/w, digits=2)
    println("querying z $CYCLES vectors took $w seconds: $speed per sec, $errors errors")

    
    # Query Y  given x,z
    ####################
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

    # Query X given z,y
    ####################
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