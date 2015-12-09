/* global angular, PUBLIC_API_BASE_URL, TIMEOUT_HTTP_CONNECTION, TIMEOUT_HTTP_CONNECTION_SERVICE_CALLS */

angular.module('buddy.services', [])

.factory ('loginService', function($http, $state , $rootScope, User, $sessionStorage, $localStorage){
    return {
        logIn: function(user_pin, callback){
            console.log("loginService.logIn > pin "+user_pin);
            console.log("loginService.logIn > token "+$sessionStorage.securityToken);
            $http.post(
                PUBLIC_API_BASE_URL+"/login",
                {
                    pin: user_pin,
                    token: $sessionStorage.securityToken
                },
                { timeout: TIMEOUT_HTTP_CONNECTION }
                        
            ).then(function (response){
                
                var resData = response.data;
                
                console.log(JSON.stringify(resData));
                
                if (resData.status===true && resData.dataResponse.logout!==true){
                    
                    var user=new User();
                    user.setLogged(true);
                    user.setName(response.data.dataResponse.name);
                    user.setLastName(response.data.dataResponse.lastName);
                    user.setNumber(response.data.dataResponse.number);
                    
                    $sessionStorage.user=user;
                    
                    $rootScope.$broadcast('user:updated');
                    
                    return callback("ok");
                    
                }else if(resData.status===true && resData.dataResponse.logout===true){
                    
                    $sessionStorage.user=null;
                    return callback("ko");
                    
                }else{
                    
                    return callback("ko");
                    
                }
                                
            },function (error){
                
                console.log(JSON.stringify(error));
                callback("ko");
                
            });
            
        },
        logOut: function(){
            
            buddyLog("loginService.logOut");
            $sessionStorage.user = null;
            if($state.current.name !== "login"){
                
                $state.transitionTo("login");
                
            }
            
        },
        checkTokenId: function (user_id, user_token, callback){
            
            //simulate http request!
            var response = {data:{is_valid_token:"yes"}};
            
            setTimeout(function(){
                buddyLog("loginService.checkTokenId > " + JSON.stringify(response.data.is_valid_token));
                callback(response.data.is_valid_token);
            },2000);
            
        },
        getTokenId: function (callback){
            
            $http.get(
                PUBLIC_API_BASE_URL+"/request-token",
                { timeout: TIMEOUT_HTTP_CONNECTION_SERVICE_CALLS }
            ).then(function (response){
                
                console.log("loginService.getTokenId > OK");
                console.log(JSON.stringify(response.data));
                
                if(typeof response !== "undefined" && response.data.status === true){
                    
                    $sessionStorage.securityToken = response.data.dataResponse.securityToken;
                    //console.log("loginService.getTokenId > " + JSON.stringify(response));
                    callback("ok");
                    
                }else{
                    
                    $sessionStorage.securityToken = "";
                    //console.log("loginService.getTokenId > " + JSON.stringify(response));
                    callback("ko");
                    
                }

            },function (error){
                
                console.log("loginService.getTokenId > ERROR");
                console.log("loginService.getTokenId > " + JSON.stringify(error));
                callback("ko");

            });
        }        
    };
})
.factory ('templateService', function($http, $state , $rootScope, User, $sessionStorage){
    return {
        getInfo: function(template_name , callback){
            
            console.log("templateService.getTemplateInfo > temaplate_name: "+template_name);
            
            if(typeof $sessionStorage.user !== "undefined" && $sessionStorage.user !== null){
                
                if(template_name === "buddy_home"){

                    $http.get(
                    
                        PUBLIC_API_BASE_URL+"/home?token="+$sessionStorage.securityToken,
                        { timeout: TIMEOUT_HTTP_CONNECTION_SERVICE_CALLS }
                    
                    ).then(function (response){

                        var resData = response.data;

                        console.log("templateService.getInfo > OK");
                        console.log(JSON.stringify(resData));
                        callback(response.data);
                        
                    },function (error){

                        console.log("templateService.getInfo > ERROR");
                        console.log(JSON.stringify(error));
                        callback(null);

                    });
                
                }
                
            }
            
            
        }
        
    };
})
.factory('userService', function ($rootScope, User, $sessionStorage, $localStorage){
    
    var userFunctions = {
        
        deleteUser: function (){
            
            buddyLog("userService.deleteUser > REQUESTED DELETE USER");
            $sessionStorage.user = null;
            buddyLog("userService.deleteUser > " + "success");
            $rootScope.$broadcast('user:updated');
            
        },
        getUser : function (callback){
            
            if(typeof $sessionStorage.user !== "undefined" && $sessionStorage.user !== "" && $sessionStorage.user !== null){
                
                callback($sessionStorage.user);
                
            }else{
                
                callback(null);
                
            }
            
        },
        storeUser : function (user, callback){
            
            if(typeof user === User && user !== "" && user !== null){
                
                $sessionStorage.user = user;
                $rootScope.$broadcast('user:updated');
                callback("ok");
                
            }else{
                
                $sessionStorage.user = null;
                $rootScope.$broadcast('user:updated');
                callback("ko");
                
            }
                        
        },
        updateAvatar: function (avatar, callback){
            
            if(typeof $sessionStorage.user !== "undefined" && $sessionStorage.user !== "" && $sessionStorage.user !== null){
                
                $sessionStorage.user.setAvatar(avatar);
                $rootScope.$broadcast('user:updated');
                callback("ok");
                
            }else{
                
                callback("ko");
                
            }
                        
        },
        saveLanguage: function(language, callback){
            
            if(typeof $localStorage !== "undefined"){
                $localStorage.language=language;
                callback("ok");
            }else{
                callback("ko");
            }
            
        }
    };

    return userFunctions;
    
});