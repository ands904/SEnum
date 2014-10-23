object TiffImageConfigForm: TTiffImageConfigForm
  Left = 120
  Top = 181
  BorderStyle = bsToolWindow
  Caption = #1055#1072#1088#1072#1084#1077#1090#1088#1099' '#1088#1077#1076#1072#1082#1090#1080#1088#1086#1074#1072#1085#1080#1103' '#1092#1072#1081#1083#1072' *.tiff'
  ClientHeight = 275
  ClientWidth = 551
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  OnKeyDown = FormKeyDown
  OnPaint = FormPaint
  PixelsPerInch = 96
  TextHeight = 13
  object EnumPanel: TPanel
    Left = 0
    Top = 0
    Width = 551
    Height = 57
    Align = alTop
    TabOrder = 0
    Visible = False
    object Label1: TLabel
      Left = 144
      Top = 11
      Width = 158
      Height = 13
      Caption = #1057' '#1082#1072#1082#1086#1081' '#1089#1090#1088#1072#1085#1080#1094#1099' '#1085#1091#1084#1077#1088#1086#1074#1072#1090#1100':'
    end
    object Label2: TLabel
      Left = 109
      Top = 34
      Width = 193
      Height = 13
      Caption = #1053#1086#1084#1077#1088' '#1087#1077#1088#1074#1086#1081' '#1085#1091#1084#1077#1088#1091#1077#1084#1086#1081' '#1089#1090#1088#1072#1085#1080#1094#1099':'
    end
    object FirstEnumPageEdit: TEdit
      Left = 315
      Top = 7
      Width = 121
      Height = 21
      TabOrder = 0
      Text = '0'
    end
    object FirstPageNumEdit: TEdit
      Left = 315
      Top = 30
      Width = 121
      Height = 21
      TabOrder = 1
      Text = '0'
    end
  end
  object OkCancelPanel: TPanel
    Left = 0
    Top = 236
    Width = 551
    Height = 39
    Align = alBottom
    TabOrder = 1
    Visible = False
    DesignSize = (
      551
      39)
    object OkButton: TButton
      Left = 199
      Top = 8
      Width = 75
      Height = 25
      Anchors = []
      Caption = 'Ok'
      TabOrder = 0
      OnClick = OkButtonClick
    end
    object CancelButton: TButton
      Left = 309
      Top = 8
      Width = 75
      Height = 25
      Anchors = []
      Caption = #1054#1090#1084#1077#1085#1072
      ModalResult = 2
      TabOrder = 1
    end
  end
  object DeletePanel: TPanel
    Left = 0
    Top = 114
    Width = 551
    Height = 57
    Align = alTop
    TabOrder = 2
    Visible = False
    object Label3: TLabel
      Left = 117
      Top = 12
      Width = 186
      Height = 13
      Caption = #1053#1086#1084#1077#1088' '#1087#1077#1088#1074#1086#1081' '#1091#1076#1072#1083#1103#1077#1084#1086#1081' '#1089#1090#1088#1072#1085#1080#1094#1099':'
    end
    object Label4: TLabel
      Left = 18
      Top = 37
      Width = 285
      Height = 13
      Caption = #1053#1086#1084#1077#1088' '#1087#1086#1089#1083#1077#1076#1085#1077#1081' '#1091#1076#1072#1083#1103#1077#1084#1086#1081' '#1089#1090#1088#1072#1085#1080#1094#1099' ('#1074#1082#1083#1102#1095#1080#1090#1077#1083#1100#1085#1086'):'
    end
    object DeleteStartEdit: TEdit
      Left = 315
      Top = 7
      Width = 121
      Height = 21
      TabOrder = 0
      Text = '0'
    end
    object DeleteEndEdit: TEdit
      Left = 315
      Top = 30
      Width = 121
      Height = 21
      TabOrder = 1
      Text = '0'
    end
  end
  object ReplacePanel: TPanel
    Left = 0
    Top = 57
    Width = 551
    Height = 57
    Align = alTop
    TabOrder = 3
    Visible = False
    object Label5: TLabel
      Left = 111
      Top = 12
      Width = 195
      Height = 13
      Caption = #1053#1086#1084#1077#1088' '#1087#1077#1088#1074#1086#1081' '#1079#1072#1084#1077#1085#1103#1077#1084#1086#1081' '#1089#1090#1088#1072#1085#1080#1094#1099':'
    end
    object Label6: TLabel
      Left = 11
      Top = 37
      Width = 294
      Height = 13
      Caption = #1053#1086#1084#1077#1088' '#1087#1086#1089#1083#1077#1076#1085#1077#1081' '#1079#1072#1084#1077#1085#1103#1077#1084#1086#1081' '#1089#1090#1088#1072#1085#1080#1094#1099' ('#1074#1082#1083#1102#1095#1080#1090#1077#1083#1100#1085#1086'):'
    end
    object ReplaceStartEdit: TEdit
      Left = 315
      Top = 7
      Width = 121
      Height = 21
      TabOrder = 0
      Text = '0'
    end
    object ReplaceEndEdit: TEdit
      Left = 315
      Top = 30
      Width = 121
      Height = 21
      TabOrder = 1
      Text = '0'
    end
  end
  object FilenamePanel: TPanel
    Left = 0
    Top = 171
    Width = 551
    Height = 42
    Align = alTop
    TabOrder = 4
    object Label7: TLabel
      Left = 8
      Top = 14
      Width = 63
      Height = 13
      Caption = #1048#1084#1103' '#1092#1072#1081#1083#1072': '
    end
    object SelectFileSpeedButton: TSpeedButton
      Left = 518
      Top = 9
      Width = 23
      Height = 22
      Caption = '...'
      OnClick = SelectFileSpeedButtonClick
    end
    object FilenameEdit: TEdit
      Left = 74
      Top = 10
      Width = 439
      Height = 21
      TabOrder = 0
    end
  end
  object OpenDialog1: TOpenDialog
    Filter = 
      #1042#1089#1077' '#1086#1073#1088#1072#1073#1072#1090#1099#1074#1072#1077#1084#1099#1077' '#1092#1072#1081#1083#1099'|*.doc;*.xls;*.pdf;*.tif;*.tiff;*.jpg;*.' +
      'jpeg|'#1060#1072#1081#1083#1099' Excel *.xls|*.xls|'#1060#1072#1081#1083#1099' Word *.doc|*.doc|'#1060#1072#1081#1083#1099' Word *' +
      '.pdf|*.pdf|'#1056#1072#1089#1090#1088#1086#1074#1099#1077' '#1088#1080#1089#1091#1085#1082#1080' *.tif|*.tif;*.tiff|'#1056#1072#1089#1090#1088#1086#1074#1099#1077' '#1088#1080#1089#1091#1085#1082 +
      #1080' *.jpg|*.jpg;*.jpeg'
    Options = [ofHideReadOnly, ofFileMustExist, ofEnableSizing]
    Left = 40
    Top = 224
  end
  object Timer1: TTimer
    Enabled = False
    Interval = 50
    OnTimer = Timer1Timer
    Left = 96
    Top = 224
  end
end
