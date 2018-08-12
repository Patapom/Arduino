Ibles.package("Ibles.views", "Ibles.models");

Ibles.siteNotificationTemplate = "<div class=\"alert alert-warning alert-dismissible site-notification\" role=\"alert\">";
Ibles.siteNotificationTemplate += "   <button type=\"button\" data-dismiss=\"alert\" class=\"close notice-close\" aria-label=\"Close\"><span>&times;<\/span><\/button>";
Ibles.siteNotificationTemplate += "   <div><< message1 >> << message2 >> << message3 >></div>";
Ibles.siteNotificationTemplate += '</div>';

Ibles.siteNotificationMobileTemplate = "<div class=\"alert alert-warning alert-dismissible site-notification\" role=\"alert\">";
Ibles.siteNotificationMobileTemplate += "   <button type=\"button\" data-dismiss=\"alert\" class=\"close notice-close\" aria-label=\"Close\"><span>&times;<\/span><\/button>";
Ibles.siteNotificationMobileTemplate += "   <div><< message1 >></div>";
Ibles.siteNotificationMobileTemplate += '</div>';

Ibles.views.NotificationView = Backbone.View.extend({

    events: {
        "click .close": "close"
    },

    template: _.template(Ibles.siteNotificationTemplate),

    initialize: function(){
        this.render();
    },

    close: function(){
        this.$('.site-notification').fadeOut(200);
        this.setClosedCookie();

    },

    setClosedCookie:function(){
        $.cookie('SiteNotificationClosed', 'true', {path: '/', expires: 7});
    },

    render: function(){
        this.$el.html(this.template(this.model.toJSON()));
        head.load("https://cdn.instructables.com/static/gtm/css/site_notifications.css");
        return this.el;
    }

});

Ibles.views.MobileNotificationView = Ibles.views.NotificationView.extend({

    template: _.template(Ibles.siteNotificationMobileTemplate)

});

Ibles.models.SiteNotification = Backbone.Model.extend({

    defaults:{
        message1: "",
        message2: "",
        message3: "",
        startDate: "",
        endDate: "",
        type: ""
    }

});

Ibles.showSiteNotification = function(model){
    var site_notification_model = new Ibles.models.SiteNotification(model);

    if( Ibles.isMobile() ){
        var mobile_site_notification_view = new Ibles.views.MobileNotificationView({model: site_notification_model});
        if( $('#explore-container').length ){
            $('#main-header').append(mobile_site_notification_view.$el);
        } else {
            $('.content').prepend(mobile_site_notification_view.$el);
        }
    } else {
        var site_notification_view = new Ibles.views.NotificationView({model: site_notification_model});
        $('body').prepend(site_notification_view.$el);
    }

};