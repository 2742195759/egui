/**
 * @file
 *
 * @author Dalian University of Technology
 *
 * @section LICENSE
 *
 * Copyright (C) 2010 Dalian University of Technology
 *
 * This file is part of EDUGUI.
 *
 * EDUGUI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * EDUGUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EDUGUI; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * All rights reserved.
**/
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include<time.h>
#include<sys/types.h>
#include<unistd.h>
#include <string.h>

# include "right_click_menu.h"

/* 右键菜单 */
# define MENU_NUM 5

/* 右键菜单 */
static struct button * menu_button[MENU_NUM];
static si_t menu_show = 0;
struct shortcut* sh_temp_ptr = NULL;
struct shortcut* sh_new_ptr = NULL;
/*桌面应用图标数(app_number)*/
char desktop_path[]="/home/wangfei/egui/debug/samples/Desktop/shortcut/";
char temp_path[100]={0};

struct window* save_window = NULL;
struct text_line* save_text_line = NULL;
struct button* save_button = NULL;
int save_or_rename_flag=0;
char save_file_name[100]={0};

void create_new_folder(){ 
    si_t i=0;
    byte_t str[100]="/home/wang/egui/_build/MAIN/NEW FOLDER";
    byte_t cpy[100]="/home/wang/egui/_build/MAIN/NEW FOLDER";
    byte_t temp_str[100];
    while(access(str,0)!=-1){//access函数是查看文件是不是存在
    	strcpy(str,cpy);
    	i++;
    	sprintf(temp_str, "%d", i);
    	strcat(str,temp_str);
    }
    if (mkdir(str,0777)) {//如果不存在就用mkdir函数来创建
    	printf("creat folder failed!!!");
    }
}

void create_new_file(){
	/* 申请快捷方式 */
	struct shortcut* sh = shortcut_init(2);
	/* 申请失败 */
	if(sh == NULL)
		application_exit();
	sh->is_real=1;



	vector_push_back(&sh_desktop_vector, sh, sizeof(struct shortcut));
	
	sh_new_ptr = vector_at(&sh_desktop_vector, app_number);
	sh_new_ptr->is_real=1;
	shortcut_set_is_text_visiable(sh_new_ptr,1);
	shortcut_set_text(sh_new_ptr,"new");
	shortcut_set_img_path(sh_new_ptr,"/home/wangfei/egui/resource/icons/file_icon2/txt_1.bmp");
	shortcut_set_img_normal_path(sh_new_ptr,"/home/wangfei/egui/resource/icons/file_icon2/txt_1.bmp");
	shortcut_set_img_select_path(sh_new_ptr,"/home/wangfei/egui/resource/icons/file_icon2/txt_2.bmp");
	strcpy(sh_new_ptr->app_name, "editerbasic");
	strcpy(sh_new_ptr->app_path, "/home/wangfei/egui/_bulid/debug/samples/");
	strcpy(sh_new_ptr->link_file_path,"/home/wangfei/me");
	sh_new_ptr->is_real=0;
	sh_new_ptr->callback=shortcut_callback;
	int num=1;
	while(1){
		struct point p;
		p.x=num/area_num_y;
		p.y=num%area_num_y;
		num++;
		if(reset_shortcut_bound(sh_new_ptr,p)==1)
			break;
	}
	
	app_number++;

	object_attach_child(OBJECT_POINTER(Desktop_im), OBJECT_POINTER(sh_new_ptr));
}

//获得目标快捷方式指针
void find_running_shortcut(){
	int find_num=0;  //查找正在作用的桌面图标
	for(find_num=0; find_num < app_number; find_num++){
		sh_temp_ptr = vector_at(&sh_desktop_vector, find_num);
		if(sh_temp_ptr->flag==1)
			break;
	}
}
//当前区域是否存在快捷方式
int is_area_having(struct point p){
	for(int i=0;i<area_num_x;i++)
		for(int j=0;j<area_num_y;j++)
			if(p.x>= area_pptr[i][j].x && p.x<= area_pptr[i][j].x+area_pptr[i][j].width && p.y>= area_pptr[i][j].y && p.y<= area_pptr[i][j].y+area_pptr[i][j].height){
				if(flag_pptr[i][j])
					return 1;
				return 0;
			}
	return 0;
}



int change_file_content(char* file,char* src,char* des){
	char file2[100]={0};
	strcpy(file2,desktop_path);
	strcat(file2,"temp22");
    FILE *in=fopen(file,"r");
    FILE *out=fopen(file2,"w");  /*out是充当临时文件作用*/
    char buffer[1024];
	memset(buffer , 0, sizeof(char)*1024);
    if(!in)
    {
        printf("cann't open file\n");
        exit(1);
    }
    if(!out)
    {
        printf("cann't create file\n");
        exit(1);
    }
        /*开始复制*/
    while(fgets(buffer,1024,in))
    {
       //if(ch=='a') ch='p';
	   char* ptr = strstr(buffer,src);
	   if(ptr!=NULL){
			strcpy(buffer,src);
			strcat(buffer,des);
			strcat(buffer,"\n");
	   }
       fputs(buffer,out);
	   memset(buffer , 0, sizeof(char)*1024);
    }
    fclose(in);  
	fclose(out);  
    unlink(file); /*删除test.txt*/
	chmod(file2, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH);
    rename(file2,file); /*改名*/
	return 0;
}


si_t rename_window_ok_button_callback(void* btn, void* msg)
{
    union message* m = (union message*)msg;
	switch(m->base.type)
	{
    case MESSAGE_TYPE_MOUSE_SINGLE_CLICK:
		sprintf(save_file_name, "%s", text_line_get_buf(save_text_line));
		if(save_or_rename_flag==2){//重命名
			if(sh_temp_ptr && sh_temp_ptr->is_real==0){
				shortcut_set_text(sh_temp_ptr,save_file_name);
				change_file_content(sh_temp_ptr->link_file_path,"NAME:",save_file_name);
			}
			else if(sh_temp_ptr && sh_temp_ptr->is_real==1){
				shortcut_set_text(sh_temp_ptr,save_file_name);
				//pid_t pid;
				//if((pid = fork()) == 0){
					char new_name[100]={0};
					strcpy(new_name,desktop_path);
					strcat(new_name,save_file_name);
					//execl("/bin/mv","mv",sh_temp_ptr->link_file_path,new_name,NULL);
					//chmod(new_name, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH);
					rename(sh_temp_ptr->link_file_path,new_name);
					strcpy(sh_temp_ptr->link_file_path,new_name);			
				//}
			}
		}
		else if(save_or_rename_flag==1){//新建文件
			shortcut_set_text(sh_new_ptr,save_file_name);
			char new_name[100]={0};
			strcpy(new_name,desktop_path);
			strcat(new_name,save_file_name);
			pid_t pid;
			if((pid = fork()) == 0){

				execl("/bin/touch","touch",new_name,NULL);
				
				//FILE *out=fopen(new_name,"w");  /*out是充当临时文件作用*/
				//fclose(out);
				chmod(new_name, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH);
			}
			strcpy(sh_new_ptr->link_file_path,new_name);	
			sh_new_ptr->is_real=1;
		}
		/*
		if((pid = fork()) == 0)
        {
            switch(save_or_rename_flag)
            {
                case 0:
                    execl("/bin/touch","touch",save_file_name,NULL);
                    break;
                case 1:
                    execl("/bin/mkdir","mkdir",save_file_name,NULL);
                    break;
                case 2:
					execl("/bin/mv","mv",temp_path,save_file_name,NULL);
                    break;
                default:
                    break;
            }
            exit(0);
        }
        usleep(100000);
		*/
		
        application_del_window(save_window);
		save_window = NULL;
		save_text_line = NULL;
		save_button = NULL;
		sh_new_ptr = NULL;
		break;
    default:
        button_default_callback(btn, msg);
        return 0;
        break;
	}
	//desktop_flush();
    return 0;
}

si_t pop_window()
{
    switch(save_or_rename_flag)
    {
        case 0:
            save_window = window_init("save_directory_window");
            break;
        case 1:
            save_window = window_init("save_file_window");
            break;
        case 2:
            save_window = window_init("rename_window");
            break;
        default:
            break;
     }
    if(NULL == save_window)
    {
        //EGUI_PRINT_ERROR("failed to init save window");
		application_exit();
        return -1;
    }
    window_set_bounds(save_window,200, 200, 300, 60);
    window_set_color(save_window, NULL, &ground_purple);
    save_text_line = text_line_init(100, 0);
    if(NULL == save_text_line)
    {
        //EGUI_PRINT_ERROR("failed to init save_text_line on save window");
        application_exit();
        return -1;
    }
    text_line_set_bounds(save_text_line, 5, 5, 230, 50);
    text_line_set_placeholder(save_text_line, "input file name");
    text_line_set_color(save_text_line, &heavy_blue, NULL, &heavy_blue );
    save_button = button_init("OK");
    if(NULL == save_button)
	{
	    //EGUI_PRINT_ERROR("failed to init ok button on save window");
        application_exit();
        return -1;
	}
    button_set_bounds(save_button, 240, 10, 50, 40);
    button_set_color(save_button, &ground_purple, &heavy_blue );
    button_set_font(save_button, FONT_MATRIX_12);
	save_button->callback = rename_window_ok_button_callback;
    object_attach_child(OBJECT_POINTER(save_window), OBJECT_POINTER(save_button));
    object_attach_child(OBJECT_POINTER(save_window), OBJECT_POINTER(save_text_line));
    application_add_window(Desktop_w, save_window);
    return 0;

}



si_t
right_click_menu_NEW_FOLDER
(void * bt,
 void * msg)
{
    switch(message_get_type(msg))
    {
    	case MESSAGE_TYPE_MOUSE_PRESS:
            //create_new_folder();
    	    break;

        default:
            button_default_callback(bt, msg);
            break;
    }
    return 0;
}

si_t
right_click_menu_NEW_FILE
(void * bt,
 void * msg)
{
    switch(message_get_type(msg))
    {
    	case MESSAGE_TYPE_MOUSE_PRESS:
			save_or_rename_flag=1;
            create_new_file();
			pop_window();
			desktop_flush();
			right_click_menu_cancel();
    	    break;

        default:
            button_default_callback(bt, msg);
            break;
    }
    return 0;
}

si_t
right_click_menu_FLUSH
(void * bt,
 void * msg)
{
    switch(message_get_type(msg))
    {
    	case MESSAGE_TYPE_MOUSE_PRESS:
    	    desktop_flush();
    	    break;

        default:
            button_default_callback(bt, msg);
            break;
    }
    return 0;
}
si_t
right_click_menu_rename
(void * bt,
 void * msg)
{
	union message* m = (union message*)msg;
    switch(message_get_type(msg))
    {
    	case MESSAGE_TYPE_MOUSE_PRESS:
			if(sh_temp_ptr!=NULL){
    	    	save_or_rename_flag=2;
				pop_window();
				desktop_flush();
				right_click_menu_cancel();
			}
    	    break;

        default:
            button_default_callback(bt, msg);
            break;
    }
	
    return 0;
}

si_t shortcut_open(struct shortcut* sh_ptr){
	shortcut_set_img_path(sh_ptr, sh_ptr->img_normal_path);
	sh_ptr->flag=0;
	pid_t id;
	id = fork();
	char* full_path=(char*)malloc(strlen(sh_ptr->app_path)+strlen(sh_ptr->app_name)+1);
	char* act=(char*)malloc(2+strlen(sh_ptr->app_name)+1);
	strcpy(full_path,sh_ptr->app_path);
	strcat(full_path,sh_ptr->app_name);
	act[0]='.',act[1]='/';
	strcat(act,sh_ptr->app_name);
	if(id == 0){
		if(strstr(sh_ptr->app_name,"image_view")!=NULL && sh_ptr->is_real==1)
			execl(full_path,act,sh_ptr->link_file_path,NULL);
		else if(strstr(sh_ptr->app_name,"editerbasic")!=NULL && sh_ptr->is_real==1)
			execl(full_path,act,sh_ptr->link_file_path,NULL);
		else if(strstr(sh_ptr->app_name,"file_browser")!=NULL && sh_ptr->is_real==1)
			execl(full_path,act,sh_ptr->link_file_path,NULL);
		else if(strstr(sh_ptr->app_name,"NULL")!=NULL && sh_ptr->is_real==1)
			;
		else
			execl(full_path,act,NULL);
	}
}

si_t right_click_menu_open(void * bt,void * msg){
	union message* m = (union message*)msg;
    switch(message_get_type(msg))
    {
    	case MESSAGE_TYPE_MOUSE_PRESS:
			if(sh_temp_ptr!=NULL){
    	    	shortcut_open(sh_temp_ptr);
				right_click_menu_cancel();
			}
    	    break;

        default:
            button_default_callback(bt, msg);
            break;
    }
	
    return 0;	

}

si_t right_click_menu_init() {
	si_t i = 0;
	for(i=0;i<MENU_NUM;i++)
		menu_button[i]=NULL;
	
	//第一个邮件按钮（新建文件夹）
	menu_button[0] = button_init("NEW FOLDER");
	/* 申请失败 */
	if(menu_button[0] == NULL)
	{
		application_exit();
		return -1;
	}
	window_set_color(menu_button[0], NULL, &light_green);
    	button_set_bounds(menu_button[0], 0, -30, 0, 0);
	menu_button[0]->callback = right_click_menu_NEW_FOLDER;    


	//第二个邮件按钮（新建文件）
	menu_button[1] = button_init("NEW FILE");
	/* 申请失败 */
	if(menu_button[1] == NULL)
	{
		application_exit();
		return -1;
	}
	window_set_color(menu_button[1], NULL, &light_green);
    	button_set_bounds(menu_button[1], 0, -30, 0, 0);
	menu_button[1]->callback = right_click_menu_NEW_FILE;    

	

	//第三个邮件按钮（打开）
	menu_button[2] = button_init("open");
	/* 申请失败 */
	if(menu_button[2] == NULL)
	{
		application_exit();
		return -1;
	}
	window_set_color(menu_button[2], NULL, &light_green);
    	button_set_bounds(menu_button[2], 0, -30, 0, 0);
	menu_button[2]->callback = right_click_menu_open;    


	//第四个邮件按钮（重命名）
	menu_button[3] = button_init("rename");
	/* 申请失败 */
	if(menu_button[3] == NULL)
	{
		application_exit();
		return -1;
	}
	window_set_color(menu_button[3], NULL, &light_green);
    	button_set_bounds(menu_button[3], 0, -30, 0, 0);
	menu_button[3]->callback = right_click_menu_rename;    


	//第五个邮件按钮（刷新）
	menu_button[4] = button_init("FLUSH");
	/* 申请失败 */
	if(menu_button[4] == NULL)
	{
		application_exit();
		return -1;
	}
	window_set_color(menu_button[4], NULL, &light_green);
    	button_set_bounds(menu_button[4], 0, -30, 0, 0);
	menu_button[4]->callback = right_click_menu_FLUSH;    
	/* 添加button */   
	for(i=0;i<MENU_NUM;i++)
		object_attach_child(OBJECT_POINTER(Desktop_im), OBJECT_POINTER(menu_button[i]));


	
	/* 右键菜单不显示 */	
	menu_show = 0;	
}



void right_click_menu_handle(union message* msg) {
	sh_temp_ptr = NULL;
	find_running_shortcut();
	if(sh_temp_ptr==NULL || is_area_having(msg->base.cursor_position)==0)
		sh_temp_ptr = NULL;
	si_t i=0;
	if(menu_show == 0){
		struct point* point_ptr=message_get_cursor_position(msg);
		si_t x=point_ptr->x, y=point_ptr->y;
		for(i=0; i<MENU_NUM; i++){
			button_set_bounds(menu_button[i], x, y-30, 100, 30);
	    		y+=30;
	    	}
		
		/* 所有图标重绘 */
    		
		for(i=0; i<MENU_NUM; i++){
			button_repaint(menu_button[i]);
			button_show(menu_button[i]);
	    	}
	    	
		/* 右键菜单显示 */	
		menu_show = 1;
	}
	else{
		for(i=0;i<MENU_NUM;i++){
			button_set_bounds(menu_button[i],0, -30, 0, 0);
		}
	
		desktop_flush();
		
		for(i=0; i<MENU_NUM; i++){
			button_repaint(menu_button[i]);
			button_show(menu_button[i]);
	    	}
		
		/* 右键菜单不显示 */
		menu_show = 0;		
	}
}

void right_click_menu_cancel() {
		si_t i;
		for(i=0;i<MENU_NUM;i++){
			button_set_bounds(menu_button[i],0, -30, 0, 0);
		}
		
		desktop_flush();
			
		for(i=0; i<MENU_NUM; i++){
			button_repaint(menu_button[i]);
			button_show(menu_button[i]);
	    	}
	    		    	
		/* 右键菜单不显示 */
		menu_show = 0;	
}

void desktop_flush(){
	/* 桌面刷新重绘 */
	image_view_reshow(Desktop_im);
  	/* 所有图标重绘 */				
	for(int i=0;i<app_number;i++){
		struct shortcut* sh = vector_at(&sh_desktop_vector, i);
		shortcut_repaint(sh);
		shortcut_show(sh);
	}
	if(save_window!=NULL){
		window_repaint(save_window);
		window_show(save_window);
	}
	if(save_text_line!=NULL){
		text_line_repaint(save_text_line);
		text_line_show(save_text_line);
	}
	if(save_button!=NULL){
		button_repaint(save_button);
		button_show(save_button);
	}
}
