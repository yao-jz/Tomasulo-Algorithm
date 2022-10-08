import tkinter
import time
from tkinter import ttk
import re
file = open("fibonacci.txt","r").read().split("\n")

reg_status = [[i,0,1,0] for i in range(32)]

win = tkinter.Tk()
win.title("Tomasolo demo")
win.geometry("1200x800+200+20")

padlabel1= ttk.Label(win,text=" ",width=5)
padlabel1.grid(row=1,column=0)

memorylabel = ttk.Label(win, text="Memory",borderwidth=5)
memorylabel.grid(row=1,column=1)
memorytree = ttk.Treeview(win,show='headings',height=50)
memorytree.grid(row=2,column=1)
memorytree["columns"] = ("Memory", "Value")
memorytree.column("Memory", width=50)
memorytree.column("Value", width=100)
memorytree.heading("Memory", text="Memory")
memorytree.heading("Value", text="Value")
# for i in range(30):
# 	memorytree.insert("", i,  values=(str(i),"0"))

padlabel1= ttk.Label(win,text=" ",width=5)
padlabel1.grid(row=1,column=2)

regfilelabel = ttk.Label(win, text="Regfile", borderwidth=5)
regfilelabel.grid(row=1,column=3)
regfiletree = ttk.Treeview(win,show='headings',height=30)
regfiletree.grid(row=2,column=3)
regfiletree["columns"] = ("Reg", "Value","Valid","ReorderNum")
regfiletree.column("Reg", width=50)
regfiletree.column("Value", width=50)
regfiletree.column("Valid", width=50)
regfiletree.column("ReorderNum", width=80)
regfiletree.heading("Reg", text="Reg")
regfiletree.heading("Value", text="Value")
regfiletree.heading("Valid", text="Valid")
regfiletree.heading("ReorderNum", text="ReorderNum")
for i in range(30):
	regfiletree.insert("", i,  values=(str(i),"0","1","0"))

padlabel1= ttk.Label(win,text=" ",width=5)
padlabel1.grid(row=1,column=4)


roblabel = ttk.Label(win, text="ROB", borderwidth=5)
roblabel.grid(row=1,column=5)
robtree = ttk.Treeview(win,show='headings',height=30)
robtree.grid(row=2,column=5)
robtree["columns"] = ("reorderBuf", "busy","execUnit","instr","instrStatus","result","storeAddress","valid")
robtree.column("reorderBuf", width=80)
robtree.column("busy", width=50)
robtree.column("execUnit", width=80)
robtree.column("instr", width=100)
robtree.column("instrStatus", width=150)
robtree.column("result", width=50)
robtree.column("storeAddress", width=80)
robtree.column("valid", width=50)
robtree.heading("reorderBuf", text="reorderBuf")
robtree.heading("busy", text="busy")
robtree.heading("execUnit", text="execUnit")
robtree.heading("instr", text="instr")
robtree.heading("instrStatus", text="instrStatus")
robtree.heading("result", text="result")
robtree.heading("storeAddress", text="storeAddress")
robtree.heading("valid", text="valid")

padlabel1= ttk.Label(win,text=" ",width=5)
padlabel1.grid(row=1,column=6)

reslabel = ttk.Label(win, text="Reservation", borderwidth=5)
reslabel.grid(row=1,column=7)
restree = ttk.Treeview(win,show='headings',height=6)
restree.grid(row=2,column=7)
restree["columns"] = ("reservation", "busy","Vj","Vk","Qj","Qk","TimeLeft","Instr","ReorderNum")
restree.column("reservation", width=80)
restree.column("busy", width=50)
restree.column("Vj", width=80)
restree.column("Vk", width=80)
restree.column("Qj", width=80)
restree.column("Qk", width=80)
restree.column("TimeLeft", width=80)
restree.column("Instr", width=100)
restree.column("ReorderNum", width=100)
restree.heading("reservation", text="reservation")
restree.heading("busy", text="busy")
restree.heading("Vj", text="Vj")
restree.heading("Vk", text="Vk")
restree.heading("Qj", text="Qj")
restree.heading("Qk", text="Qk")
restree.heading("TimeLeft", text="TimeLeft")
restree.heading("Instr", text="Instr")
restree.heading("ReorderNum", text="ReorderNum")
# for i in range(6):
# 	restree.insert("", i,  values=(str(i),str(i),str(i),str(i),str(i),str(i),str(i),str(i),str(i)))
line_cnt = 0
for l in file:
	if l[:6] == "Cycles":
		break
	p = r'memory\[(\d+)\]=(-?\d+)'
	result = re.findall(p,l)[0]
	memorytree.insert("",result[0],values=(result[0],result[1]))
	line_cnt += 1

ressta = r'Reservation stations:'
ressub = r'Reservation station (\d+)'
resvj = r'Vj = (-?\d+)'
resvk = r'Vk = (-?\d+)'
resqj = r'Qj = \'(.+?)\''
resqk = r'Qk = \'(.+?)\''
restime = r'ExTimeLeft = (\d+)'
resrb = r'RBNum = (\d+)'
resinstr = r'instr = (-?\d+)'
reorderbuf = r'Reorder buffers:'
reorderbuf_pattern = r'Reorder buffer (\d+): instr (-?\d+)  executionUnit \'(.+?)\'  state (.+?)  valid (\d)  result (-?\d+) storeAddress (\d+)'
regis = r'Register result status:'
regis_pattern = r'Register (\d+): waiting for reorder buffer number (\d+)'
mem = r'Memory:'
mem_pattern = r'memory\[(\d+)\] = (-?\d+)'
reg = r'Registers:'
reg_pattern = r'regFile\[(\d+)\] = (-?\d+)'

pclabel = ttk.Label(win, text="pc: 16",borderwidth=5)
pclabel.grid(row=0,column=1)
cyclelabel = ttk.Label(win, text="cycle: 0", borderwidth=5)
cyclelabel.grid(row=0,column=2)

def f():
	global line_cnt
	if re.findall(r'halting machine', file[line_cnt]) != []:
		haltlabel = ttk.Label(win, text="HALT",borderwidth=5)
		haltlabel.grid(row=0,column=3)
		return
	while len(re.findall(r'Cycles',file[line_cnt])) == 0:
		line_cnt += 1
	cyclelabel = ttk.Label(win, text="cycle: " + re.findall(r'Cycles: (\d+)', file[line_cnt])[0], borderwidth=5)
	print("cycle: " + re.findall(r'Cycles: (\d+)', file[line_cnt])[0])
	cyclelabel.grid(row=0,column=2)
	line_cnt += 1
	pclabel = ttk.Label(win, text="pc: "+ re.findall(r'pc=(\d+)',file[line_cnt])[0],borderwidth=5)
	pclabel.grid(row=0,column=1)
	print("pc: "+ re.findall(r'pc=(\d+)',file[line_cnt])[0])
	line_cnt += 2
	for i in restree.get_children():
		restree.delete(i)
	while True:
		if len(re.findall(reorderbuf, file[line_cnt])) != 0:
			break
		rsindex = re.findall(ressub,file[line_cnt])[0]

		try:
			rsvj = re.findall(resvj,file[line_cnt])[0]
		except:
			rsvj = 0
		try:
			rsvk = re.findall(resvk,file[line_cnt])[0]
		except:
			rsvk = 0
		try:
			rsqj = re.findall(resqj,file[line_cnt])[0]
		except:
			rsqj = 0
		try:
			rsqk = re.findall(resqk,file[line_cnt])[0]
		except:
			rsqk = 0
		rstime = re.findall(restime,file[line_cnt])[0][0]
		rsrb = re.findall(resrb,file[line_cnt])[0]
		rsinstr = re.findall(resinstr,file[line_cnt])[0]
		restree.insert("", int(rsindex), values=(rsindex, 1, rsvj, rsvk, rsqj, rsqk, rstime, rsinstr, rsrb))
		line_cnt += 1
	line_cnt += 1
	for i in robtree.get_children():
		robtree.delete(i)
	while True:
		if len(re.findall(regis, file[line_cnt])) != 0:
			break
		info = re.findall(reorderbuf_pattern,file[line_cnt])[0]
		robtree.insert("", int(info[0]),values=(info[0],1,info[2],info[1],info[3],info[5],info[6],info[4]))
		line_cnt += 1
	line_cnt += 1
	for i in reg_status:
		i[2]=1
		i[3]=0
	while True:
		if len(re.findall(mem, file[line_cnt])) != 0:
			break
		info = re.findall(regis_pattern,file[line_cnt])[0]
		reg_status[int(info[0])][2]=0
		reg_status[int(info[0])][3]=info[1]
		line_cnt += 1
	line_cnt += 1
	for i in memorytree.get_children():
		memorytree.delete(i)
	while True:
		if len(re.findall(reg, file[line_cnt])) != 0:
			break
		info = re.findall(mem_pattern,file[line_cnt])[0]
		memorytree.insert("", int(info[0]),values=(info[0],info[1]))
		line_cnt += 1
	line_cnt += 1
	while True:
		if len(re.findall(r'Cycles', file[line_cnt])) != 0:
			break
		info = re.findall(reg_pattern,file[line_cnt])[0]
		reg_status[int(info[0])][1]=info[1]
		line_cnt += 1
	for i in regfiletree.get_children():
		regfiletree.delete(i)
	for i in range(32):
		regfiletree.insert("", i, values=tuple(reg_status[i]))


button=ttk.Button(win,text="下一步",command=f)
button.grid(row=0,column=0)



win.mainloop()