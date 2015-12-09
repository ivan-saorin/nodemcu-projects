/* global angular, $j, PUBLIC_API_BASE_URL */

angular.module('buddy.controllers', [])

.controller("LoginController", function($scope, $state , $ionicLoading , loginService){
    
    $scope.loginFail = false;
    var $pinObj = $j("#buddy_main_login_password");
    
    $pinObj.on("focus" , function(){
        $scope.loginFail = false;
    });
    
    $scope.login=function(){
        
        buddyLog("LoginController > Login");
        var pin = $pinObj.val();
        
        var loading_templateUrl = "templates/loader_login.html";
        
        $ionicLoading.show({
            //templateUrl: loading_templateUrl
        });
        
        loginService.logIn(pin , function (res){
            
            buddyLog("$scope.login > " + JSON.stringify(res.data));
            if(res === "ok"){
                
                $pinObj.val("");
                $ionicLoading.hide();
                $state.go('buddy.home');
                
            }else{
                
                $ionicLoading.hide();
                $scope.loginFail = true;
                
                setTimeout(function(){
                    $scope.loginFail = false;
                },1000);
                
            }
            
        });
        
        
    };

})
.controller('HomeController', function($scope, $state, templateService, User, $sessionStorage, $ionicHistory, $sce){
    
	$scope.init = function(){
            
            console.log("HomeController.init");
            
            $ionicHistory.clearHistory(); //it's ROOT
            $scope.pageCompleted='';
            
            $scope.getBlocks = function() {
                
                templateService.getInfo("buddy_home" , function(result){

                    $scope.pageCompleted='';
                    
                    if(typeof result !== "undefined" && result !== null && result.status === true ){

                        $scope.items=[];
                        var i = 0;
                        angular.forEach(result.dataResponse,function(item){
                            item.image = $sce.trustAsResourceUrl(PUBLIC_API_BASE_URL + item.image);
                            $scope.items.push(item);
                            i++;
                        });
                        $scope.pageCompleted='ok';

                    }else{

                        $scope.pageCompleted='ko';

                    }

                });
                
            };

            $scope.$on('$ionicView.afterEnter', function(){
                
                console.log("HomeController.init > after enter event list");	
                $scope.getBlocks();
                
            });

            
	};
    
})
.controller('LogoController',function(){
    
})
.controller('NavController', function(
        $scope, $ionicSideMenuDelegate, $sessionStorage, loginService, 
        $localStorage, User, $state, $rootScope,  $ionicPopup,
        $ionicHistory, $timeout, $translate,$sce){
            
    $scope.userName="";
    $scope.userLastName="";
    $scope.userId=null;
    $scope.avatar="";
    
    console.log($sessionStorage.user);
    
    if($sessionStorage.user!==null){
    
        $scope.userName=$sessionStorage.user.getName();
        $scope.userLastName=$sessionStorage.user.getLastName();
        $scope.userId=$sessionStorage.user.getId();
        $scope.avatar="";
        buddyLog($sessionStorage.user.getAvatar());
        if(typeof $sessionStorage.user.getAvatar()!=="undefined" && $sessionStorage.user.getAvatar()!==null && $sessionStorage.user.getAvatar()!==""){
            $scope.avatar=$sce.trustAsResourceUrl($sessionStorage.user.getAvatar());
        }
        
    }
    
    $scope.Nav_logOut = function(){
        
        loginService.logOut();
        
    };
    
    $scope.toggleLeft = function(){
        
        $ionicSideMenuDelegate.toggleLeft();
        
    };
    
    $scope.$on('user:updated', function(event,data){
        
        if($sessionStorage.user!==null){
            
            $scope.userName=$sessionStorage.user.getName();
            $scope.userLastName=$sessionStorage.user.getLastName();
            $scope.userId=$sessionStorage.user.getId();
            $scope.avatar="";
            buddyLog($sessionStorage.user.getAvatar());
            if(typeof $sessionStorage.user.getAvatar()!=="undefined" && $sessionStorage.user.getAvatar()!==null && $sessionStorage.user.getAvatar()!==""){
                $scope.avatar=$sce.trustAsResourceUrl($sessionStorage.user.getAvatar());
            }
            
        }else{
            
            $scope.userId=null;
            
        }
    });


    $scope.logout = function (manual){
        return;
    };

	
})






.controller('ExampleAController', function($scope, $state, $ionicPopup){
    
    
    
})
.controller('ExampleBController', function($scope, $state, $ionicPopup){
    
    
    
});