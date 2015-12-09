/* global angular */

angular.module('buddy.models', [])

.factory('User', function(){
		
    function User(){
        this.name="";
        this.lastName="";
        this.id="";
        this.number="";
        this.logged="false";
        this.tokenId="";
        this.avatar="";
    }
	
    User.prototype = {
        setName: function(name){
            this.name=name;
        },
        getName: function (){
            return this.name;
        },
        setLastName:function (lastName){
            this.lastName=lastName;
        },
        getLastName:function(){
            return this.lastName;
        },
        setId: function (id){
            this.id=id;
        },
        getId: function(){
            return this.id;
        },
        setNumber: function (number){
            this.number=number;
        },
        getNumber: function(){
            return this.number;
        },
        setLogged: function(logged){
            this.logged=logged;
        },
        isLogged: function(){
            return this.logged;
        },
        setTokenId: function (tokenId){
            this.tokenId=tokenId;
        },
        getTokenId: function(){
            return this.tokenId;
        },
        setAvatar: function(avatar){
            this.avatar=avatar;
        },
        getAvatar: function(){
            return this.avatar;
        }
    };
	
    return User;
	
})
.factory('InputUserData', function (){
    function InputUserData (email, password, name, lastName, title, organization, website, about){
        this.email=email;
        this.password=password;
        this.name=name;
        this.lastName=lastName;
        this.title=title;
        this.organization=organization;
        this.website=website;
        this.about=about;
    }
	
    InputUserData.prototype = {
        getEmail: function (){
            return this.email;
        },
        getPassword: function (){
            return this.password;
        },
        getName: function (){
            return this.name;
        },
        getLastName: function(){
            return this.lastName;
        },
        getTitle: function (){
            return this.title;
        },
        getOrganization: function (){
            return this.organization;
        },
        getWebsite: function (){
            return this.website;
        },
        getAbout: function (){
            return this.about;
        }
    };
	
    return InputUserData;

});