/* global angular, StatusBar, cordova */

// Ionic Starter App

var OPEN_WEATHER_API_KEY = "12303a21ff7fe1aec498f71950c28711";

var PUBLIC_API_BASE_URL="http://95.211.178.226:9080/BuddyBank";
var TIMEOUT_HTTP_CONNECTION=20000; //  20 seconds
var TIMEOUT_HTTP_CONNECTION_SERVICE_CALLS=3000; //  2 seconds, for hidden operations


function buddyLog(data){
    //console.log(data);
}
$j = $;


angular.module('buddy',[
            'ionic',
            'ngStorage',
            'pascalprecht.translate',
            'buddy.controllers',
            'buddy.services',
            'buddy.models'
])
.run(function($ionicPlatform, $state, $rootScope, $sessionStorage, $translate, $ionicLoading, $localStorage, User , loginService) {

    $ionicPlatform.ready(function() {

        // Hide the accessory bar by default (remove this to show the accessory bar above the keyboard
        // for form inputs)
        if(window.cordova && window.cordova.plugins.Keyboard) {
            cordova.plugins.Keyboard.hideKeyboardAccessoryBar(true);
        }
        
        if(window.StatusBar) {
            StatusBar.styleDefault();
        }

        var routesFree = ['login'];

        $translate.use("IT");

        $ionicLoading.show({
            //templateUrl: loading_templateUrl
        });

        console.log("REQUESTING TOKEN");
        loginService.getTokenId(function(res){

            if(res === "ok"){

                $ionicLoading.hide();

                $rootScope.$on('$stateChangeStart', function (event, next, current) {

                    var isFree=false;
                    var i=0;
                    while(isFree===false && i<routesFree.length){
                        if(routesFree[i]===next.name){
                            isFree=true;
                        }
                        i++;
                    }
                    //console.log(JSON.stringify($sessionStorage.user));
                    var isLogged="false";
                    if(typeof $sessionStorage.user!=="undefined" && typeof $sessionStorage.user.isLogged!=="undefined"){
                        isLogged=$sessionStorage.user.isLogged();
                    }

                    if(!isFree && isLogged==="false" && current.name !== "login"){

                        event.preventDefault();
                        $state.transitionTo("login");

                    }

                });

                document.addEventListener('deviceready', function () {

                });

            }else{

                $ionicLoading.hide();
                alert("Something wrong here...check your internet connection please");

            }

        });

  });

})
.config(['$stateProvider', '$urlRouterProvider', '$httpProvider', '$ionicConfigProvider',
function($stateProvider, $urlRouterProvider, $httpProvider) {

    // Enable cross domain calls
    $httpProvider.defaults.useXDomain = true;
    // Remove the header used to identify ajax call  that would prevent CORS from working
    delete $httpProvider.defaults.headers.common['X-Requested-With'];
    $httpProvider.defaults.withCredentials = true;
    $httpProvider.defaults.headers.post['Content-Type'] = 'application/x-www-form-urlencoded;charset=utf-8';

    /**
        * The workhorse; converts an object to x-www-form-urlencoded serialization.
        * @param {Object} obj
        * @return {String}
    */
    var param = function(obj) {

        var query = '', name, value, fullSubName, subName, subValue, innerObj, i;

        for(name in obj) {

            value = obj[name];

            if(value instanceof Array) {
                for(i=0; i<value.length; ++i) {
                    subValue = value[i];
                    fullSubName = name + '[' + i + ']';
                    innerObj = {};
                    innerObj[fullSubName] = subValue;
                    query += param(innerObj) + '&';
                }
            }
            else if(value instanceof Object) {
                for(subName in value) {
                    subValue = value[subName];
                    fullSubName = name + '[' + subName + ']';
                    innerObj = {};
                    innerObj[fullSubName] = subValue;
                    query += param(innerObj) + '&';
                }
            }
            else if(value !== undefined && value !== null)
                query += encodeURIComponent(name) + '=' + encodeURIComponent(value) + '&';
        }

        return query.length ? query.substr(0, query.length - 1) : query;
    };

    $httpProvider.defaults.transformRequest = [function(data) {
        return angular.isObject(data) && String(data) !== '[object File]' ? param(data) : data;
    }];


    $stateProvider
	.state('login', {
            url: "/login",
            templateUrl: "templates/login.html",
            controller: 'LoginController'
	})
        .state('buddy', {
            url: "/buddy",
            abstract: true,
            templateUrl: "templates/menu_main.html",
            controller: 'NavController'
        })
	.state('buddy.home', {
            url: "/home",
            views: {
                'menuContent' :{
                    templateUrl: "templates/home.html",
                    controller: 'HomeController'
                }
            }
	})
	.state('buddy.exampleA', {
            url:"/home/exampleA",
            views: {
                'menuContent' :{
                    templateUrl: "templates/exampleA.html",
                    controller: 'ExampleAController'
                }
            }
	})
	.state('buddy.exampleB', {
            url:"/home/exampleB",
            views: {
                'menuContent' :{
                    templateUrl: "templates/exampleB.html",
                    controller: 'ExampleBController'
                }
            }
	})
	.state('logo', {
            url: "/logo",
            templateUrl: "templates/logo.html",
            controller: 'LogoController'
	});

	$urlRouterProvider.otherwise('/login');
}]);
