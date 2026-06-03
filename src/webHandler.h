/**
   MyHandler.h
   @autor    Bruno Merz

   @version  1.0

*/

#pragma once

#include <Arduino.h>
#include "ESPAsyncWebServer.h"


const char fsstyle1[] PROGMEM =  R"=====(
    body {
      font-family: sans-serif;
      background-color: #aaaaaa;
      display: flex;
      max-width : 500px;
      flex-flow: column;
      align-items: center;
    }
    h1,h2 {
      color: #e1e1e1;
      text-shadow: 2px 2px 2px black;
    }
    li {
      background-color: #feb1e2;
      list-style-type: none;
      margin-bottom: 10px;
      padding: 2px 5px 1px 0;
      box-shadow: 5px 5px 5px rgba(0,0,0,0.7);
    }
    li a:first-child, li b {
      background-color: #8f05a5;
      font-weight: bold;
      color: white;
      text-decoration:none;
      padding: 2px 5px;
      text-shadow: 2px 2px 1px black;
      cursor:pointer;
    }
    li strong {
      color: red;
    }
    input {
      height:35px;
      font-size:14px;
    }
    label + a {
      text-decoration: none;
    }
    h1 + main {
      display: flex;
    }  
    aside {
      display: flex;
      flex-direction: column;
      padding: 0.2em;
    }
    #left {
      align-items:flex-end;
      text-shadow: 0.5px 0.5px 1px #757474;
    }
    .note {
      background-color: #fecdee;
      padding: 0.5em;
      margin-top: 1em;
      text-align: center;
      max-width: 320px;
      border-radius: 0.5em;
    }
    [type=submit] {
      height:40px; 
      font-size: 14px;
    }
    .buttons {
      background-color: #353746;
      text-align:center;
      line-height: 22px;
      color:#FFFFFF;
      width:200px;
      height: 32px;
      padding:1px;
      border:2px solid #FFFFFF;
      font-size:14px;
      border-radius:15px;
      cursor: pointer;
    }
    form [title] {
      background-color: #353746;
      font-size: 14px;
      width: 150px;
    }
    )=====";
    
     // Teil 2
    const char fsstyle2[] PROGMEM =  R"=====(
    nav {
      display: flex;
      align-items: baseline;
      justify-content: space-between;
    }
    #left {
      align-items:flex-end;
      text-shadow: 0.5px 0.5px 1px #757474;
    }
    #cr {
      font-weight: bold;
      cursor:pointer;
      font-size: 1.5em;
    }
    #up {
      width: auto; 
    }
    .note {
      background-color: #fecdee;
      padding: 0.5em;
      margin-top: 1em;
      text-align: center;
      max-width: 320px;
      border-radius: 0.5em;
    }
    .no {
      display: none;
    }
    
    [value*=Format] {
      margin-top: 1em;
      box-shadow: 5px 5px 5px rgba(0,0,0,0.7);
    }
    [name="group"] {
      display: none;
    }
    [name="group"] + label {
      font-size: 1.5em;
      margin-right: 5px;
    }
    [name="group"] + label::before {
      content: "\002610";
    } 
    [name="group"]:checked + label::before {
      content: '\002611\0027A5';
    }
    .sanduhrcontainer {
      position: absolute;
        top: 130px;
        left: 180px;
    }
    .sanduhr {
        position: absolute;
          margin: 0px auto;   
    }
    )=====";


class WebHandler {
  public:
    WebHandler(void);
    bool init(void);
    void webRequests(void);
    void start(void);
    AsyncWebServer *getServer(void);

  private:
    AsyncWebServer *webServer;
};